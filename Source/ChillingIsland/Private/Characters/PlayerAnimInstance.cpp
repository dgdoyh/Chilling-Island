#include "Characters/PlayerAnimInstance.h"
#include "Characters/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Begin Play
void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Get player pawn
	PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());

	// Get player's movement component
	if (PlayerCharacter)
	{
		PlayerMovementComponent = PlayerCharacter->GetCharacterMovement();
	}
}

// Tick
void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (PlayerMovementComponent)
	{
		// Get player's current speed
		RunningSpeed = UKismetMathLibrary::VSizeXY(PlayerMovementComponent->Velocity);

		// Get whether player is falling or not
		IsFalling = PlayerMovementComponent->IsFalling();
	}
}
