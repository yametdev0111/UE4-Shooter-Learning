// Shooter, All Rights Reserved


#include "Components/ShooterWeaponComponent.h"

#include "Animations/AnimUtils.h"
#include "Animations/ShooterEquipFinishedAnimNotify.h"
#include "Animations/ShooterReloadFinishedAnimNotify.h"
#include "Player/ShooterBaseCharacter.h"
#include "Weapon/ShooterBaseWeapon.h"

DEFINE_LOG_CATEGORY_STATIC(LogShooterWeaponComponent, All, All);

static constexpr int32 NUM_WEAPONS_REQUIRED = 2;

UShooterWeaponComponent::UShooterWeaponComponent(
)
{
	PrimaryComponentTick.bCanEverTick = false;

}


void UShooterWeaponComponent::StartShooting(
)
{
	if (!CanShoot()) {
		return;
	}
	CurrentWeapon->StartShooting();
}

void UShooterWeaponComponent::StopShooting(
)
{
	if (!CurrentWeapon) {
		return;
	}
	CurrentWeapon->StopShooting();
}

void UShooterWeaponComponent::EquipNextWeapon(
)
{
	if (!CanEquip()) {
		return;
	}

	int32 NextWeaponIndex = (CurrentWeaponIndex + 1) % Weapons.Num();
	EquipWeapon(NextWeaponIndex);
}

void UShooterWeaponComponent::ReloadWeapon(
)
{
	if (!CanReload()) {
		return;
	}
	ChangeClip();
}

void UShooterWeaponComponent::BeginPlay(
)
{
	Super::BeginPlay();

	checkf(WeaponData.Num() == NUM_WEAPONS_REQUIRED, TEXT("Character must have %d weapons! You have: %d"), NUM_WEAPONS_REQUIRED, WeaponData.Num());

	checkf(EquipAnimMontage, TEXT("EquipAnimMontage is not set!"));

	for (auto const &SingleWeaponData : WeaponData) {
		checkf(SingleWeaponData.WeaponClass, TEXT("WeaponClass is not set for all weapons!"));
		checkf(SingleWeaponData.ReloadAnimMontage, TEXT("ReloadAnimMontage is not set for all weapons!"));
	}

	InitAnimations();

	CurrentWeaponIndex = 0;
	SpawnWeapons();

	EquipWeapon(CurrentWeaponIndex);
}

void UShooterWeaponComponent::EndPlay(
	EEndPlayReason::Type const EndPlayReason
)
{
	StopShooting();

	CurrentWeapon = nullptr;

	for (AShooterBaseWeapon *Weapon : Weapons) {
		if (Weapon) {
			Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			Weapon->Destroy();
		}
	}
	Weapons.Empty();

	Super::EndPlay(EndPlayReason);
}

void UShooterWeaponComponent::SpawnWeapons(
)
{
	UWorld *World = GetWorld();
	if (!World) {
		return;
	}

	ACharacter *Character = Cast<ACharacter>(GetOwner());
	if (!Character) {
		return;
	}

	USkeletalMeshComponent *SkeletalMesh = GetOwnerSkeletalMesh();
	if (!SkeletalMesh) {
		return;
	}

	for (auto &SingleWeaponData : WeaponData) {
		AShooterBaseWeapon *Weapon = World->SpawnActor<AShooterBaseWeapon>(SingleWeaponData.WeaponClass);
		if (!Weapon) {
			UE_LOG(LogShooterWeaponComponent, Warning, TEXT("Couldn't spawn weapon %s for %s"), *SingleWeaponData.WeaponClass->GetName(), *Character->GetName());
			continue;
		}
		Weapon->SetOwner(Character);
		Weapons.Add(Weapon);
		AttachWeaponToSocket(Weapon, SkeletalMesh, WeaponArmorySocketName);
		Weapon->OnEmptyClip.AddUObject(this, &UShooterWeaponComponent::OnEmptyClip);
	}

}

void UShooterWeaponComponent::AttachWeaponToSocket(
	AShooterBaseWeapon *Weapon,
	USceneComponent *SceneComponent,
	FName const &SocketName
)
{
	if (!Weapon || !SceneComponent) {
		return;
	}
	FAttachmentTransformRules AttachmentRules{ EAttachmentRule::SnapToTarget, false };
	Weapon->AttachToComponent(SceneComponent, AttachmentRules, SocketName);
}

void UShooterWeaponComponent::EquipWeapon(
	int32 WeaponIndex
)
{
	if (!Weapons.IsValidIndex(WeaponIndex)) {
		return;
	}

	USkeletalMeshComponent *SkeletalMesh = GetOwnerSkeletalMesh();
	if (!SkeletalMesh) {
		return;
	}

	// Find selected weapon and it's reload animation
	AShooterBaseWeapon *Weapon = Weapons[WeaponIndex];
	FWeaponData *SelectedWeaponData = nullptr;
	if (Weapon) {
		SelectedWeaponData = WeaponData.FindByPredicate(
			[&Weapon](FWeaponData const &Data) {
				return Data.WeaponClass == Weapon->GetClass();
			}
		);
	}
	UAnimMontage *WeaponReloadMontage = SelectedWeaponData ? SelectedWeaponData->ReloadAnimMontage : nullptr;

	StopShooting();

	if (CurrentWeapon != Weapon) { // If equipping not current weapon - put old one on the back
		AttachWeaponToSocket(CurrentWeapon, SkeletalMesh, WeaponArmorySocketName); // Place current weapon on the back
	}

	AttachWeaponToSocket(Weapon, SkeletalMesh, WeaponEquippedSocketName); // Equip selected weapon

	CurrentWeapon = Weapon;
	CurrentReloadAnimMontage = WeaponReloadMontage;
	CurrentWeaponIndex = WeaponIndex;

	bActionAnimInProgress = true;
	PlayAnimMontage(EquipAnimMontage);
}

void UShooterWeaponComponent::PlayAnimMontage(
	UAnimMontage *Montage
)
{
	ACharacter *Character = Cast<ACharacter>(GetOwner());
	if (!Character) {
		return;
	}

	Character->PlayAnimMontage(Montage);
}

USkeletalMeshComponent *UShooterWeaponComponent::GetOwnerSkeletalMesh(
)
{
	ACharacter *Character = Cast<ACharacter>(GetOwner());
	if (!Character) {
		return nullptr;
	}
	return Character->GetMesh();
}

void UShooterWeaponComponent::InitAnimations(
)
{
	InitEquipAnimation();

	for (FWeaponData &SingleWeaponData : WeaponData) {
		InitReloadAnimation(SingleWeaponData.ReloadAnimMontage);
	}
}

void UShooterWeaponComponent::InitEquipAnimation(
)
{
	UShooterEquipFinishedAnimNotify *EquipFinishedNotify = AnimUtils::FindNotifyByClass<UShooterEquipFinishedAnimNotify>(EquipAnimMontage);
	if (EquipFinishedNotify) {
		EquipFinishedNotify->OnNotified.AddUObject(this, &UShooterWeaponComponent::OnEquipFinished);
	}
	else {
		UE_LOG(LogShooterWeaponComponent, Error, TEXT("EquipFinishedAnimNotify not set for EquipAnimMontage!"));
		checkNoEntry();
	}
}

void UShooterWeaponComponent::InitReloadAnimation(
	UAnimMontage *ReloadMontage
)
{
	UShooterReloadFinishedAnimNotify *ReloadFinishedNotify = AnimUtils::FindNotifyByClass<UShooterReloadFinishedAnimNotify>(ReloadMontage);
	if (ReloadFinishedNotify) {
		ReloadFinishedNotify->OnNotified.AddUObject(this, &UShooterWeaponComponent::OnReloadFinished);
	}
	else {
		UE_LOG(LogShooterWeaponComponent, Error, TEXT("ReloadFinishedAnimNotify not set for one of ReloadMontages!"));
		checkNoEntry();
	}
}

bool UShooterWeaponComponent::CanShoot(
) const
{
	return CurrentWeapon && !bActionAnimInProgress;
}

bool UShooterWeaponComponent::CanEquip(
) const
{
	return Weapons.Num() && !bActionAnimInProgress;
}

bool UShooterWeaponComponent::CanReload(
) const
{
	return CurrentWeapon && CurrentWeapon->CanReload() && !bActionAnimInProgress;
}

void UShooterWeaponComponent::ChangeClip(
)
{
	if (!CanReload()) {
		return;
	}

	CurrentWeapon->StopShooting();
	CurrentWeapon->ChangeClip();

	bActionAnimInProgress = true;
	PlayAnimMontage(CurrentReloadAnimMontage);
}

void UShooterWeaponComponent::OnEquipFinished(
	USkeletalMeshComponent *MeshComp
)
{
	if (GetOwnerSkeletalMesh() == MeshComp) {
		UE_LOG(LogShooterWeaponComponent, Display, TEXT("Equip Finished!"));
		bActionAnimInProgress = false;
	}
}

void UShooterWeaponComponent::OnReloadFinished(
	USkeletalMeshComponent *MeshComp
)
{
	if (GetOwnerSkeletalMesh() == MeshComp) {
		UE_LOG(LogShooterWeaponComponent, Display, TEXT("Reload Finished!"));
		bActionAnimInProgress = false;
	}
}

void UShooterWeaponComponent::OnEmptyClip(
)
{
	ChangeClip();
}
