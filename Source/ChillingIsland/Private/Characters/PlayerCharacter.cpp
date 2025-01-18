#include "Characters/PlayerCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/Item.h"
#include "Items/Equipments/Weapon.h"
#include "Animation/AnimMontage.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Disable player rotation following controller's rotation
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Rotate player to the direction of movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// Set rotation speed
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	// Create a spring arm
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	// Attach it to the root component
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;

	// Create and set up a view camera
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Add a mapping context to subsystem (IMC will be set in the engine)
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(PlayerMappingContext, 0);
		}
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	if (ActionState == EActionState::EAS_Attacking) return;
	if (Controller && (Value.GetMagnitude() != 0.f))
	{
		// Get movement vector from input
		const FVector2D MoveVector = Value.Get<FVector2D>();

		// Get player controller's yaw rotation value
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw,0.f);

		// Get directional vector of the controller
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
		// Move player based on the controller's yaw rotation
		AddMovementInput(ForwardDirection, MoveVector.Y);
		AddMovementInput(RightDirection, MoveVector.X);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	// Get looking vector from input
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	// Rotate controller based on mouse axis
	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void APlayerCharacter::Interact(const FInputActionValue& Value)
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);

	// ** Instant code **
	if (OverlappingWeapon)
	{
		// Destroy the currently equipped weapon
		if (EquippedWeapon)
		{
			EquippedWeapon->UnEquip();
		}
		
		// Equip one-handed weapon
		if (OverlappingWeapon->GetWeaponType() == EWeaponTypes::EWT_OneHandedWeapon)
		{
			OverlappingWeapon->Equip(GetMesh(), FName("RightHandSocket"));
			CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		}
		
		// Equip two-Handed weapon
		if (OverlappingWeapon->GetWeaponType() == EWeaponTypes::EWT_TwoHandedWeapon)
		{
			OverlappingWeapon->Equip(GetMesh(), FName("TwoHandsSocket"));
			CharacterState = ECharacterState::ECS_EquippedTwoHandedWeapon;
		}

		// Set equipped weapon
		EquippedWeapon = OverlappingWeapon;
	}
}

void APlayerCharacter::Attack(const FInputActionValue& Value)
{
	// Attack only when player is not occupied && equipping a weapon
	if (CanAttack())
	{
		PlayAttackMontage();

		ActionState = EActionState::EAS_Attacking;
	}
}

void APlayerCharacter::PlayAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	UAnimMontage* AttackMontage = nullptr;

	if (CharacterState == ECharacterState::ECS_EquippedOneHandedWeapon && OneHandedAttackMontage)
	{
		AttackMontage = OneHandedAttackMontage;
	}
	if (CharacterState == ECharacterState::ECS_EquippedTwoHandedWeapon && TwoHandedAttackMontage)
	{
		AttackMontage = TwoHandedAttackMontage;
	}
	
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);

		// Play random attack animation
		const int32 RandomInt = FMath::RandRange(0,1);
		FName SectionName = FName();
		
		switch (RandomInt)
		{
		case 0:
			SectionName = FName("Attack1");
			break;
		case 1:
			SectionName = FName("Attack2");
			break;
		default:
			break;
		}
		
		AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
	}
}

void APlayerCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

bool APlayerCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Bind input actions and functions
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Jump);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Interact);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Attack);
	}
}

void APlayerCharacter::Jump()
{
	if (CanJump)
	{
		Super::Jump();
		
		CanJump = false;

		// Cannot jump until it gets cooled down
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
		{
			CanJump = true;
		}, JumpCoolTime, false);
	}
}

