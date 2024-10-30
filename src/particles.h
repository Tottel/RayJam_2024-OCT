#ifndef PARTICLES_H
#define PARTICLES_H
/**********************************************************************************************
*
*   libpartikel v0.0.3 ALPHA
*   [https://github.com/dbriemann/libpartikel]
*
*
*   A simple particle system built with and for raylib, to be used as header only library.
*
*
*   FEATURES:
*       - Supports all platforms that raylib supports
*
*   DEPENDENCIES:
*       raylib >= v2.5.0 and all of its dependencies
*
*   CONFIGURATION:
*   #define LIBPARTIKEL_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*   LICENSE: zlib/libpng
*
*   libpartikel is licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software:
*
*   Copyright (c) 2017 David Linus Briemann (@Raging_Dave)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/
// Additionally to the above, a lot of modifications were made by Tottel for performance and extra functionality

// TODO This whole thing needs a rewrite to be mindful of data..

#pragma once

#include "raylib.h"
#include "raymath.h"
//#include "KShapes.h"
//#include "KMath.h"
//#include <cstdlib>

// -----------------------------------------------------------------------
// THIS MUST BE COMMENTED OUT
// You need to uncomment it in some editors to enable syntax highlighting
// in the following code.
#define LIBPARTIKEL_IMPLEMENTATION
// -----------------------------------------------------------------------

// Needed forward declarations.
//----------------------------------------------------------------------------------
typedef struct Particle Particle;
typedef struct EmitterConfig EmitterConfig;
typedef struct Emitter Emitter;
typedef struct ParticleSystem ParticleSystem;


// Function signatures (comments are found in implementation below)
//----------------------------------------------------------------------------------
float GetRandomFloat(float min, float max);
Color LinearFade(Color c1, Color c2, float fraction);

// Particle functors
bool Particle_DeactivatorAge(Particle* p);

float Particle_Decellerator_Default(Particle* p);
float Particle_Decelerator_Linear(Particle* p);
float Particle_Decelerator_Cubic(Particle* p);
float Particle_Decelerator_Sudden(Particle* p);

Emitter* Emitter_New(EmitterConfig cfg);
void Emitter_Tweak_Direction(Emitter* e, Vector2 dirNew);
void Emitter_Tweak_velocity(Emitter* e, float velMin, float velMax);
void Emitter_Tweak_External_Acceleration(Emitter* e, Vector2 accNew);
void Emitter_Tweak_Burst(Emitter* e, int burstMing, int burstMax);
void Emitter_Set_Origin(Emitter* e, Vector2 origin);
void Emitter_Start(Emitter* e);
void Emitter_Stop(Emitter* e);
void Emitter_Free(Emitter* e);
void Emitter_Burst(Emitter* e);
void Emitter_Update(Emitter* e, float dt);
void Emitter_Draw(Emitter* e);

ParticleSystem* ParticleSystem_New(void);
bool ParticleSystem_Register(ParticleSystem* ps, Emitter* emitter);
bool ParticleSystem_Deregister(ParticleSystem* ps, Emitter* emitter);
void ParticleSystem_SetOrigin(ParticleSystem* ps, Vector2 origin);
void ParticleSystem_Start(ParticleSystem* ps);
void ParticleSystem_Stop(ParticleSystem* ps);
void ParticleSystem_Burst(ParticleSystem* ps);
void ParticleSystem_Draw(ParticleSystem* ps);
void ParticleSystem_Update(ParticleSystem* ps, float dt);
void ParticleSystem_Free(ParticleSystem* p);
void ParticleSystem_CleanAndFree(ParticleSystem* p);

int ParticleSystem_GetAllActiveParticleCount(ParticleSystem* ps);


#ifdef LIBPARTIKEL_IMPLEMENTATION

#include "stdlib.h"
#include "math.h"

// Utility functions * structs.
//----------------------------------------------------------------------------------


// Min/Max pair structs for various types.
typedef struct FloatRange {
    float min;
    float max;
} FloatRange;

typedef struct IntRange {
    int min;
    int max;
} IntRange;


// EmitterConfig type.
//----------------------------------------------------------------------------------
struct EmitterConfig {
    Vector2 direction;              // Direction vector will be normalized.
    FloatRange velocity;            // The possible range of the particle velocities.
    // Velocity is a scalar defining the length of the direction vector.
    FloatRange directionAngle;      // The angle range modiying the direction vector.
    FloatRange velocityAngle;       // The angle range to rotate the velocity vector.
    FloatRange offset;              // The min and max offset multiplier for the particle origin.
    FloatRange size;
    IntRange burst;                 // The range of sudden particle bursts.
    size_t capacity;                // Maximum amounts of particles in the system.
    size_t emissionRate;            // Rate of emitted particles per second.
    Vector2 origin;                 // Origin is the source of the emitter.
    Vector2 externalAcceleration;   // External constant acceleration. e.g. gravity.
    Color Color;                    // The color the particle starts with when it spawns.
    FloatRange age;                 // Age range of particles in seconds.
    FloatRange haltTime;            // Halt time of particles in seconds.
    BlendMode blendMode;            // Color blending mode for all particles of this Emitter.
    Texture2D texture;              // The texture used as particle texture.   

    bool (*particle_Deactivator)(struct Particle*); // Pointer to a function that determines when
    // a particle is deactivated.

//float (*particle_Decellerator)(struct Particle *);
};


// Particle type.
//----------------------------------------------------------------------------------

// Particle describes one particle in a particle system.
struct Particle {
    //Vector2 origin;                 // The origin of the particle (never changes).
    //Vector2 position;               // Position of the particle in 2d space.
    float posX;
    float posY;
    float velX;
    float velY;
    float accExtX;
    float accExtY;
    float size;
    //Vector2 velocity;               // Velocity vector in 2d space.
    //Vector2 externalAcceleration;   // Acceleration vector in 2d space.
    //float originAcceleration;       // Accelerates velocity vector
    float age;                      // Age is measured in seconds.
    float ttl;                      // Ttl is the time to live in seconds.
    float haltTime;                 // After how long the particle should stop moving. Works together with halt function

    // TODO REMOVE
    bool active;                    // Inactive particles are neither updated nor drawn.

    // TODO REMOVE if not used for age
    bool (*particle_Deactivator)(struct Particle*); // Pointer to a function that determines
    // when a particle is deactivated.

    float (*particle_Decellerator)(struct Particle*);
};

// Emitter type.
//----------------------------------------------------------------------------------

// Emitter is a single (point) source emitting many particles.
struct Emitter {
    EmitterConfig config;
    float mustEmit;            // Amount of particles to be emitted within next update call.
    Vector2 offset;             // Offset holds half the width and height of the texture.
    bool isEmitting;

    Vector2* particlePositions;
    Vector2* particleVelocities;
    Vector2* particlesAccellerationExt;
    float* particleSizes;
    float* particleAges;
    float* particleTTL;
    float* particleHaltTimes;

    int activeParticles;
};

// ParticleSystem type.
//----------------------------------------------------------------------------------

// ParticleSystem is a set of emitters grouped logically
// together to achieve a specific visual effect.
// While Emitters can be used independently, ParticleSystem
// offers some convenience for handling many Emitters at once.
struct ParticleSystem {
    bool active;
    size_t length;
    size_t capacity;
    Vector2 origin;
    Emitter** emitters;
};

#endif // LIBPARTIKEL_IMPLEMENTATION


#endif