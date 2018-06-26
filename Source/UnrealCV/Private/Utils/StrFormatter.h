// Weichao Qiu @ 2017
#pragma once

/** Serialize data to plain text format */
class FStrFormatter : public FArchive
{
public:
	FString ToString() { return Str; }
	FString Str;
private:
};

inline FStrFormatter& operator<<(FStrFormatter& Ar, FVector& Vec)
{
	// Ar << Vec.X << " " << Vec.Y << " " << Vec.Z;
	Ar.Str += FString::Printf(TEXT("%.3f %.3f %.3f"), Vec.X, Vec.Y, Vec.Z);
	return Ar;
}

inline FStrFormatter& operator<<(FStrFormatter& Ar, FRotator& Rotator)
{
	Ar.Str += FString::Printf(TEXT("%.3f %.3f %.3f"), Rotator.Pitch, Rotator.Yaw, Rotator.Roll);
	return Ar;
}
