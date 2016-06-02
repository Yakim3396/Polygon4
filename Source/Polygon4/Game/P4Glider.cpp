/*
 * Polygon-4
 * Copyright (C) 2015 lzwdgc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Polygon4.h"
#include "P4Glider.h"

#include "Projectile.h"

#include "GliderMovement.h"
#include "GliderHUD.h"

#include "P4Mechanoid.h"
#include "P4MapBuilding.h"

const float k_mouse_x = 0.6;
const float k_mouse_y = 0.8;

FPowerUpProperties::FPowerUpProperties()
{
    HoverInAir = true;
    HoverAltitude = 300.0f;
    GravityScale = 0.2f;
    LiftSpring = 350000.0f;
    LiftDamp = -128.0f;
    SlowFall = false;
    FallSpeed = 100.0f;
    SelfRight = true;
    AlphaTrack = 500000.0f;
    AlphaDamp = -128.0f;
}

AP4Glider::AP4Glider()
{
	PrimaryActorTick.bCanEverTick = true;

    //AutoPossessPlayer = EAutoReceiveInput::Player0;
    //AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    VisibleComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisibleComponent"));
    VisibleComponent->SetupAttachment(RootComponent);
    VisibleComponent->SetSimulatePhysics(false);
    RootComponent = VisibleComponent;
    Body = VisibleComponent;

    EngineAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudioComponent"));
    EngineAudioComponent->SetupAttachment(RootComponent);
    //EngineAudioComponent->bOverrideAttenuation = true;

    WeaponAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WeaponAudioComponent"));
    WeaponAudioComponent->SetupAttachment(RootComponent);
    //WeaponAudioComponent->bOverrideAttenuation = true;

    static ConstructorHelpers::FObjectFinder<USoundWave> JumpSoundAsset(TEXT("SoundWave'/Game/Mods/Common/Sounds/Glider/jump.jump'"));
    if (JumpSoundAsset.Succeeded())
        JumpSound = JumpSoundAsset.Object;

    static ConstructorHelpers::FObjectFinder<USoundWave> EngineSoundAsset(TEXT("SoundWave'/Game/Mods/Common/Sounds/Glider/engine.engine'"));
    if (EngineSoundAsset.Succeeded())
        EngineSound = EngineSoundAsset.Object;

    static ConstructorHelpers::FObjectFinder<USoundWave> LightSoundAsset(TEXT("SoundWave'/Game/Mods/Common/Sounds/Weapon/light.light'"));
    if (LightSoundAsset.Succeeded())
        LightSound = LightSoundAsset.Object;

    static ConstructorHelpers::FObjectFinder<USoundWave> HeavySoundAsset(TEXT("SoundWave'/Game/Mods/Common/Sounds/Weapon/heavy.heavy'"));
    if (HeavySoundAsset.Succeeded())
        HeavySound = HeavySoundAsset.Object;

    FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FPSCamera"));
    FirstPersonCameraComponent->SetupAttachment(RootComponent);

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("TPSCameraSpringArm"));
    SpringArm->bUsePawnControlRotation = false;
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->RelativeRotation = FRotator(-15.f, 0.f, 0.f);
    SpringArm->TargetArmLength = 500.0f;
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 8.0f;

    ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TPSCamera"));
    ThirdPersonCameraComponent->bUsePawnControlRotation = false;
    ThirdPersonCameraComponent->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> EnergyShieldAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (EnergyShieldAsset.Succeeded())
    {
        EnergyShield = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnergyShield"));
        EnergyShield->SetStaticMesh(EnergyShieldAsset.Object);
        EnergyShield->SetWorldScale3D({ 6.2,6.2,6.2 });
        EnergyShield->SetupAttachment(RootComponent);
        EnergyShield->SetSimulatePhysics(false);
        EnergyShield->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        EnergyShield->SetOwnerNoSee(true);

        static ConstructorHelpers::FObjectFinder<UMaterial> EnergyShieldMat(TEXT("Material'/Game/Mods/Common/Materials/EnergyShield.EnergyShield'"));
        if (EnergyShieldMat.Succeeded())
        {
            EnergyShield->SetMaterial(0, EnergyShieldMat.Object);
        }
    }

    Body->SetSimulatePhysics(true);
    Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Body->OnComponentHit.AddDynamic(this, &AP4Glider::OnBodyHit);
    Body->OnComponentBeginOverlap.AddDynamic(this, &AP4Glider::OnBodyBeginOverlap);
    //Body->OnComponentBeginOverlap.AddDynamic(this, &AP4Glider::OnEnergyShieldBeginOverlap);
    //Body->OnComponentEndOverlap.AddDynamic(this, &AP4Glider::OnEnergyShieldEndOverlap);

    UpdateView();

    GunOffsetLeft = FVector(150.0f, -100.0f, 0.0f);
    GunOffsetRight = FVector(150.0f, 100.0f, 0.0f);
    GunOffsetTop = FVector(150.0f, 0.0f, 100.0f);

    static ConstructorHelpers::FObjectFinder<UClass> light(TEXT("Class'/Game/Mods/Common/Projectiles/SimpleProjectile.SimpleProjectile_C'"));
    if (light.Object)
        projectileLight = light.Object;
    static ConstructorHelpers::FObjectFinder<UClass> heavy(TEXT("Class'/Game/Mods/Common/Projectiles/HeavyProjectile.HeavyProjectile_C'"));
    if (heavy.Object)
        projectileHeavy = heavy.Object;

    JumpTimeout = ArmedTimedValue(1.5);
    rpmLight = ArmedTimedValue(0.1f);
    rpmHeavy = ArmedTimedValue(1.5f);
    SlopeAngle = FloatDampingValue(50);
    EnergyShieldTimer = FloatFadedValue(2);
}

void AP4Glider::BeginPlay()
{
    EngineAudioComponent->Sound = EngineSound;

    SpringArm->bInheritRoll = false;
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritYaw = true;

    Body->SetPhysicsMaxAngularVelocity(90);
    Body->SetMassOverrideInKg(NAME_None, 4000.0f);

    Super::BeginPlay();
}

void AP4Glider::Tick(float DeltaSeconds)
{
    // parent
    Super::Tick(DeltaSeconds);

    if (!Mechanoid)
        return;

    Lifetime += DeltaSeconds;

    // vars
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    static const int mouse_gap = 30;
    static const int mouse_pos_gap = 30;

    const auto loc = GetActorLocation();
    const auto rot = GetActorRotation();

    Mechanoid->x = loc.X;
    Mechanoid->y = loc.Y;
    Mechanoid->z = loc.Z;
    Mechanoid->pitch = rot.Pitch;
    Mechanoid->yaw = rot.Yaw;
    Mechanoid->roll = rot.Roll;

    // conditions
    //if (!PlayerController)
    //    return;

    // engine sound
    {
        if (GetVelocity().Size() > 150 && !EngineAudioComponent->IsPlaying())
        {
            //EngineAudioComponent->Play();
            //EngineAudioComponent->bAlwaysPlay = true;
        }
        else
        {
            //EngineAudioComponent->bAlwaysPlay = false;
        }
    }

    // timers
    JumpTimeout += DeltaSeconds;
    rpmLight += DeltaSeconds;
    rpmHeavy += DeltaSeconds;
    EnergyShieldTimer += DeltaSeconds;

    // mouse pos
    if (PlayerController)
    {
        auto Viewport = GetWorld()->GetGameViewport();
        FIntPoint ViewSize = Viewport->Viewport->GetSizeXY();
        FIntPoint Center = ViewSize / 2;
        //Center.Y -= 20;
        FVector2D Position;
        bool Focused1 = Viewport->IsFocused(Viewport->Viewport);
        bool Focused2 = Viewport->Viewport->HasFocus();
        if ((Focused1 || Focused2) && PlayerController->GetMousePosition(Position.X, Position.Y) &&
            (Position.X != Center.X || Position.Y != Center.Y))
        {
            float mouse_diff_max = 3;
            auto mouse_diff_x = fabs(MousePosition.X - Position.X);
            auto mouse_diff_y = fabs(MousePosition.Y - Position.Y);
            if (!(mouse_diff_x < mouse_diff_max && mouse_diff_y < mouse_diff_max))
            {
                MousePosition = Position;
                MousePositionRepeats = 0;
            }
            else if ((fabs(Position.X - Center.X) > mouse_pos_gap || fabs(Position.Y - Center.Y) > mouse_pos_gap))
            {
                MousePosition = Position;
                MousePositionRepeats++;
            }

            enum class MouseChange
            {
                None,
                Start,
                End,
            };
            auto NormPos = [](float &x, int len, float k)
            {
                int window = len * k;
                int pos = round(x);
                int start = (len - window) / 2.0f;
                int end = start + window;
                MouseChange changed = MouseChange::None;
                if (pos > end)
                {
                    pos = end;
                    changed = MouseChange::End;
                }
                else if (pos < start)
                {
                    pos = start;
                    changed = MouseChange::Start;
                }
                x = pos;
                return changed;
            };

            auto oldpos = Position;
            auto changed_x = NormPos(Position.X, ViewSize.X, k_mouse_x);
            auto changed_y = NormPos(Position.Y, ViewSize.Y, k_mouse_y);
            bool changed = changed_x != MouseChange::None || changed_y != MouseChange::None;

            if (changed)
            {
                //if (changed_y == MouseChange::End)
                //    Position.X++;
                Viewport->Viewport->SetMouse(Position.X, Position.Y);
            }

            if (MousePositionRepeats > 50)
            {
                auto kx = (Position.X - Center.X) / (float)Center.X;
                auto ky = (Position.Y - Center.Y) / (float)Center.Y;
                FVector2D NewPosition = Position;
                float base_k = 3;
                float base_k_x;
                float base_k_y;
                float steps_x = fabs(Position.X - Center.X) / base_k;
                float steps_y = fabs(Position.Y - Center.Y) / base_k;
                float steps = std::max(steps_x, steps_y);
                if (steps_x > steps_y)
                {
                    base_k_x = base_k;
                    base_k_y = base_k / (steps_x / steps_y);
                }
                else
                {
                    base_k_x = base_k / (steps_y / steps_x);
                    base_k_y = base_k;
                }
                NewPosition.X -= copysignf(base_k_x, kx);
                NewPosition.Y -= copysignf(base_k_y, ky);
                if ((NewPosition.X > Center.X && Position.X <= Center.X) ||
                    (NewPosition.X < Center.X && Position.X >= Center.X))
                    NewPosition.X = Center.X;
                if ((NewPosition.Y > Center.Y && Position.Y <= Center.Y) ||
                    (NewPosition.Y < Center.Y && Position.Y >= Center.Y))
                    NewPosition.Y = Center.Y;
                Viewport->Viewport->SetMouse(NewPosition.X, NewPosition.Y);

                MousePosition = NewPosition;
                Position = NewPosition;
            }

            auto HUD = Cast<AGliderHUD>(PlayerController->GetHUD());
            HUD->SetMousePosition(Position);
        }
    }

    // weapons
    if (1)
    {
        float rpm1 = 60.f / 400.0f;
        float rpm2 = 60.f / 60.0f;

        FRotator SpawnRotation = rot;
        if (PlayerController)
        {
            FVector loc;
            FVector rot;
            PlayerController->DeprojectMousePositionToWorld(loc, rot);
            SpawnRotation = rot.Rotation();
        }
        SpawnRotation.Roll = 0;

        const FVector SpawnLocationLeft = loc +SpawnRotation.RotateVector(GunOffsetLeft);
        const FVector SpawnLocationRight = loc + SpawnRotation.RotateVector(GunOffsetRight);
        const FVector SpawnLocationTop = loc + SpawnRotation.RotateVector(GunOffsetTop);

        AProjectile *p = nullptr;
        if (FireLight && projectileLight && rpmLight)
        {
            auto &loc = LeftGun ? SpawnLocationLeft : SpawnLocationRight;
            p = (AProjectile *)GetWorld()->SpawnActor(projectileLight, &loc, &SpawnRotation);
            LeftGun = !LeftGun;
            //WeaponAudioComponent->Sound = LightSound;
            //WeaponAudioComponent->Play();
            UGameplayStatics::PlaySoundAtLocation(this, LightSound, loc);
        }
        if (FireHeavy && projectileHeavy && rpmHeavy)
        {
            p = (AProjectile *)GetWorld()->SpawnActor(projectileHeavy, &SpawnLocationTop, &SpawnRotation);
            //WeaponAudioComponent->Sound = HeavySound;
            //WeaponAudioComponent->Play();
            UGameplayStatics::PlaySoundAtLocation(this, HeavySound, SpawnLocationTop);
        }
        if (p)
        {
            p->SetOwner(this);
        }
    }

    // trace
    ZTraceResults = HoverTrace();
    if (!ZTraceResults.bBlockingHit)
    {
        return;
    }

    auto CalcAngle = [](const FVector &Vector1, const FVector &Vector2 = FVector::UpVector)
    {
        float dp = FVector::DotProduct(Vector1, Vector2);
        dp /= Vector1.Size() * Vector2.Size();
        float acos_ = acosf(dp);
        float grad = (acos_ * (180 / 3.1415926));
        if (isnan(grad))
            grad = 90.0;
        return grad;
    };

    // altitude
    {
        SlopeAngle += CalcAngle(ZTraceResults.ImpactNormal);

        const float angle_max = 50.0f; // make angle - slow value (like average on last 5 values) - damping
        bool on_slope = SlopeAngle > angle_max;
        if (on_slope)
        {
            //ZTraceResults = HoverTrace(ZTraceResults.ImpactPoint);
            if (!ZTraceResults.bBlockingHit)
                return;
        }

        float Altitude = ZTraceResults.Distance;
        float g = 980;
        float force = g;
        bool applied = false;

        auto ForceUp = [this, &Altitude, &force, &applied]()
        {
            if (applied)
                return;
            applied = true;
            auto Velocity = Body->GetComponentVelocity();
            auto vel = -Velocity.Z;
            auto min_alt = std::min(PowerUpProperties.HoverAltitude, Altitude);
            min_alt = min_alt / 2;
            if (vel > min_alt)
                force += vel * 2;
        };

        if (!on_slope)
        {
            if (Altitude > PowerUpProperties.HoverAltitude + 50) // force down
            {
                force = -g;
            }
            else if (!applied && Altitude < PowerUpProperties.HoverAltitude) // force up
            {
                auto diff = PowerUpProperties.HoverAltitude - Altitude;
                //force = g - (Altitude - PowerUpProperties.HoverAltitude) * 3;
                force = g * (1 + 3 * diff / PowerUpProperties.HoverAltitude);
            }
            else
                force = 0;
        }
        else
        {
            float slope_altitude = PowerUpProperties.HoverAltitude / 3 * 2;
            if (Altitude > slope_altitude) // small down
            {
                force = (slope_altitude - Altitude) / 2;
            }
            else if (!applied && Altitude < slope_altitude) // force up
            {
                force = g - (Altitude - slope_altitude) * 2;
            }
            else
                force = 0;
        }

        ForceUp();

        Body->SetLinearDamping(1.0f);
        Body->SetAngularDamping(1.0f);

        FVector ForceVector = FVector::UpVector;
        if (on_slope)
        {
            ForceVector = ZTraceResults.ImpactNormal;
        }
        Body->AddForce(ForceVector * Body->GetMass() * force);
    }

    // Torque
    {
        FVector Torque = ZTraceResults.Distance > PowerUpProperties.HoverAltitude ? FVector::UpVector : ZTraceResults.ImpactNormal;
        if (PlayerController)
        {
            FVector2D Position;
            if (PlayerController->GetMousePosition(Position.X, Position.Y))
            {
                auto Viewport = GetWorld()->GetGameViewport();
                FIntPoint ViewSize = Viewport->Viewport->GetSizeXY();
                FIntPoint Center = ViewSize / 2;
                //Center.Y -= 20;
                if (fabs(Position.X - Center.X) > mouse_gap)
                    Position.X = (Position.X - Center.X) / (float)Center.X / k_mouse_x;
                else
                    Position.X = 0;
                if (fabs(Position.Y - Center.Y) > mouse_gap)
                    Position.Y = (Position.Y - Center.Y) / (float)Center.Y / k_mouse_y;
                else
                    Position.Y = 0;

                auto ScalePosition = [](float &p)
                {
                    bool sign = signbit(p);
                    p *= 1;
                    p = p * p;
                    if (sign && p > 0)
                        p = -p;
                };

                ScalePosition(Position.X);
                ScalePosition(Position.Y);

                float diff;
                int sign;

                // left/right
                Body->AddTorque(Position.X * GetActorUpVector() * 10000 * Body->GetMass());

                // top/bottom
                float YawAngle = CalcAngle(FVector::UpVector, GetActorForwardVector());
                diff = YawAngle - 90.0f;
                sign = 1;
                if (diff > 0)
                    sign = -1;
                diff = fabs(diff);
                auto RightVector = GetActorRightVector();
                // auto torque
                if (0);
                else if (diff > 50)
                    Body->AddTorque(sign * RightVector * (sign ? 50000 : 20000) * Body->GetMass());
                else if (diff > 25)
                    Body->AddTorque(sign * RightVector * (sign ? 5000 : 3000) * Body->GetMass());
                // control torque
                Body->AddTorque(Position.Y * RightVector * 6000 * Body->GetMass());

                // do not roll
                float RollAngle = CalcAngle(Torque, GetActorRightVector());
                diff = RollAngle - 90.0f;
                sign = 1;
                if (diff < 0)
                    sign = -1;
                diff = fabs(diff);
                if (0);
                else if (diff > 50)
                    Body->AddTorque(sign * GetActorForwardVector() * 20000 * Body->GetMass());
                else if (diff > 20)
                    Body->AddTorque(sign * GetActorForwardVector() * 10000 * Body->GetMass());
                else if (diff > 10)
                    Body->AddTorque(sign * GetActorForwardVector() * 5000 * Body->GetMass());
                else
                    Body->AddTorque(sign * GetActorForwardVector() * 500 * Body->GetMass());
            }
        }
        else
        {
            Torque = Torque ^ -GetActorUpVector();
            Torque *= 10000;
            Body->AddTorque(Torque * Body->GetMass());
        }
    }

    // energy shield
    EnergyShield->SetVisibility(EnergyShieldTimer);

    //UE_LOG(LogTemp, Warning, TEXT("Sphere radius = %f"), Body->Bounds.GetSphere().W);

    // end
}

void AP4Glider::SetupPlayerInputComponent(class UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);

    InInputComponent->BindAction("View", IE_Pressed, this, &AP4Glider::ChangeView);

    InInputComponent->BindAxis("Move", this, &AP4Glider::Move);
    InInputComponent->BindAxis("Strafe", this, &AP4Glider::Strafe);

    //InInputComponent->BindAxis("Turn", this, &AP4Glider::Turn);
	//InInputComponent->BindAxis("LookUp", this, &AP4Glider::LookUp);
    InInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    InInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

    InInputComponent->BindAction("Jump", IE_Pressed, this, &AP4Glider::Jump);
    //InInputComponent->BindAction("Jump", IE_Released, this, &AP4Glider::Jump);

    InInputComponent->BindAction("Boost", IE_Pressed, this, &AP4Glider::BoostOn);
    InInputComponent->BindAction("Boost", IE_Released, this, &AP4Glider::BoostOff);

    InInputComponent->BindAction("FireLight", IE_Pressed, this, &AP4Glider::FireLightOn);
    InInputComponent->BindAction("FireLight", IE_Released, this, &AP4Glider::FireLightOff);
    InInputComponent->BindAction("FireHeavy", IE_Pressed, this, &AP4Glider::FireHeavyOn);
    InInputComponent->BindAction("FireHeavy", IE_Released, this, &AP4Glider::FireHeavyOff);

    InInputComponent->BindAction("HideUI", IE_Pressed, this, &AP4Glider::HideUI);
}

void AP4Glider::Move(float AxisValue)
{
    if (Controller != NULL && AxisValue != 0.0f)
    {
        AxisValue *= 1000;
        if (boost)
            AxisValue *= 2;
        Body->AddForce(GetActorForwardVector() * Body->GetMass() * AxisValue);

        if (!EngineAudioComponent->IsPlaying())
            EngineAudioComponent->Play();

        //UGameplayStatics::PlaySoundAtLocation(this, EngineSound, GetActorLocation());
    }
}

void AP4Glider::Strafe(float AxisValue)
{
    if (Controller != NULL && AxisValue != 0.0f)
    {
        AxisValue *= 600;
        Body->AddForce(GetActorRightVector() * Body->GetMass() * AxisValue);

        if (!EngineAudioComponent->IsPlaying())
            EngineAudioComponent->Play();

        //UGameplayStatics::PlaySoundAtLocation(this, EngineSound, GetActorLocation());
    }
}

void AP4Glider::Jump()
{
    if (JumpTimeout)
    {
        Body->AddForce(80000 * GetActorUpVector() * Body->GetMass());
        if (JumpSound)
            UGameplayStatics::PlaySoundAtLocation(this, JumpSound, GetActorLocation());
    }
}

void AP4Glider::Turn(float AxisValue)
{
    FRotator NewRotation = GetActorRotation();
    NewRotation.Yaw += AxisValue;
    SetActorRotation(NewRotation);
}

void AP4Glider::LookUp(float AxisValue)
{
    FRotator NewRotation = GetActorRotation();
    NewRotation.Pitch += AxisValue;
    SetActorRotation(NewRotation);
}

void AP4Glider::ChangeView()
{
    GliderView = (GliderView + 1) % EGliderView::Max;
    UpdateView();
}

void AP4Glider::UpdateView()
{
    switch (GliderView)
    {
    case FPS:
    //case FPSWeapons:
        FirstPersonCameraComponent->SetActive(true);
        ThirdPersonCameraComponent->SetActive(false);
        VisibleComponent->SetOwnerNoSee(true);
        break;
    case TPS:
        FirstPersonCameraComponent->SetActive(false);
        ThirdPersonCameraComponent->SetActive(true);
        VisibleComponent->SetOwnerNoSee(false);
        break;
    }
}

FHitResult AP4Glider::HoverTrace(float Altitude) const
{
    static const FName HoverTraceTag("HoverTrace");
    //GetWorld()->DebugDrawTraceTag = HoverTraceTag;

    FCollisionQueryParams TraceParams(HoverTraceTag, true, this);
    TraceParams.bTraceAsyncScene = true;
    //TraceParams.TraceTag = HoverTraceTag;

    auto begin = GetActorLocation();
    auto end = begin;
    end.Z -= Altitude;

    FHitResult Hit(ForceInit);
    GetWorld()->LineTraceSingleByChannel(
        Hit,
        begin,
        end,
        ECollisionChannel::ECC_Visibility,
        TraceParams);
    return Hit;
}

FHitResult AP4Glider::HoverTrace(FVector Vector, float Altitude) const
{
    static const FName HoverTraceTag("HoverTrace");
    //GetWorld()->DebugDrawTraceTag = HoverTraceTag;

    FCollisionQueryParams TraceParams(HoverTraceTag, true, this);
    TraceParams.bTraceAsyncScene = true;
    //TraceParams.TraceTag = HoverTraceTag;

    auto begin = GetActorLocation();
    auto end = (Vector - begin) * Altitude;

    FHitResult Hit(ForceInit);
    GetWorld()->LineTraceSingleByChannel(
        Hit,
        begin,
        end,
        ECollisionChannel::ECC_Visibility,
        TraceParams);
    return Hit;
}

void AP4Glider::HideUI()
{
    UIHidden = !UIHidden;

    auto PlayerController = Cast<APlayerController>(GetController());
    auto HUD = Cast<AGliderHUD>(PlayerController->GetHUD());
    HUD->SetVisible(!UIHidden);
}

void AP4Glider::SetMechanoid(polygon4::detail::Mechanoid* Mechanoid)
{
    this->Mechanoid = Mechanoid;
    Configuration = Mechanoid->getConfiguration();
    Glider = Configuration->glider;
}

void AP4Glider::OnBodyHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor != this && OtherComp != NULL)
    {
        EnergyShieldTimer.start();
    }
}

void AP4Glider::OnBodyBeginOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor == nullptr)
        return;
    auto Building = Cast<AP4Building>(OtherActor);
    if (Building && CanEnterBuilding())
    {
        Destroy();
    }
}

void AP4Glider::OnEnergyShieldBeginOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor != this && OtherComp != NULL)
    {
        EnergyShieldTimer.start();
    }
}

void AP4Glider::OnEnergyShieldEndOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}
