#pragma once

UENUM(BlueprintType)
enum class EWeaponTypes : uint8
{
	EWT_OneHandedWeapon UMETA(DisplayName = "One-Handed Weapon"),
	EWT_TwoHandedWeapon UMETA(DisplayName = "Two-Handed Weapon")
};