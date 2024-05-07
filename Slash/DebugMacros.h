#pragma once
#include "DrawDebugHelpers.h"

#define DRAW_SPHERE(Location) if (GetWorld()) DrawDebugSphere(GetWorld(),Location,25.f,12,FColor::Red,true);
#define DRAW_SPHERE_COLOR(Location,Color) if (GetWorld()) DrawDebugSphere(GetWorld(),Location,25.f,12,Color,false,5.f);
#define DRAW_SPHERE_SingleFrame(Location) if (GetWorld()) DrawDebugSphere(GetWorld(),Location,10.f,12,FColor::Red,false,-1.f);
#define DRAW_VECTOR(StartLocation, EndLocation) if (GetWorld()) \
	{ \
		DrawDebugLine(GetWorld(),StartLocation,EndLocation,FColor::Red,true,-1.0f,0,1.0f); \
		DrawDebugPoint(GetWorld(),EndLocation,15.0f,FColor::Red,true); \
	}

#define DRAW_VECTOR_SingleFrame(StartLocation, EndLocation) if (GetWorld()) \
	{ \
		DrawDebugLine(GetWorld(),StartLocation,EndLocation,FColor::Red,false,-1.0f,0,1.0f); \
		DrawDebugPoint(GetWorld(),EndLocation,15.0f,FColor::Red,false,-1.f); \
	}