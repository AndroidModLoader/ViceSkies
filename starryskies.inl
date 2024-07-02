#define AMOUNT_OF_SIDESTARS 100

enum eStarSides : uint8_t
{
    SSide_Left = 0,
    SSide_Right,
    SSide_Front,
    SSide_Back,
    SSide_Up,

    SSidesCount
};

float StarCoorsX[SSidesCount][AMOUNT_OF_SIDESTARS], StarCoorsY[SSidesCount][AMOUNT_OF_SIDESTARS], StarSizes[SSidesCount][AMOUNT_OF_SIDESTARS]; // 5 - sides
float fSmallStars, fMiddleStars, fBiggestStars, fBiggestStarsSpawnChance;
CVector PositionsTable[SSidesCount] =
{
    { 100.0f,  0.0f,   10.0f}, // Left
    {-100.0f,  0.0f,   10.0f}, // Right
    {   0.0f,  100.0f, 10.0f}, // Front
    {   0.0f, -100.0f, 10.0f}, // Back
    {   0.0f,  0.0f,   95.0f}, // Up
};

extern "C" void StarrySkies_Patch(float intensity)
{
    CVector ScreenPos, WorldPos, WorldStarPos;
    float SZ, SZX, SZY;

    for(int side = 0; side < SSidesCount; ++side)
    {
        WorldPos = PositionsTable[side] + *CamPos;
        for(int i = 0; i < AMOUNT_OF_SIDESTARS; ++i)
        {
            WorldStarPos = WorldPos;
            SZ = StarSizes[side][i];
            switch(side)
            {
                case SSide_Left:
                case SSide_Right:
                    WorldStarPos.y -= StarCoorsX[side][i];
                    WorldStarPos.z += StarCoorsY[side][i];
                    break;

                case SSide_Front:
                case SSide_Back:
                    WorldStarPos.x -= StarCoorsX[side][i];
                    WorldStarPos.z += StarCoorsY[side][i];
                    break;

                default:
                    WorldStarPos.x += StarCoorsX[side][i];
                    WorldStarPos.y += StarCoorsY[side][i];
                    break;
            }

            if(CalcScreenCoors(&WorldStarPos, &ScreenPos, &SZX, &SZY, false))
            {
                if(!hasJPatch15) SZX /= *ms_fAspectRatio;
                uint8_t brightness = (1.0 - 0.015f * (rand() & 0x1F)) * intensity;
                RenderBufferedOneXLUSprite(ScreenPos, SZX * SZ, SZY * SZ,
                                           brightness, brightness, brightness, 255, 1.0f / ScreenPos.z, 255);
            }
        }
    }
}

inline float RandomIt(float min, float max)
{
    return (((float)rand()) / (float)RAND_MAX) * (max - min) + min;
}

inline float ClampFloat(float value, float min, float max)
{
    if(value > max) return max;
    if(value < min) return min;
    return value;
}

void InitializeThoseStars()
{
    fSmallStars = ClampFloat(cfg->GetFloat("SmallestStarsSize", 0.15f), 0.03f, 2.5f);
    fMiddleStars = ClampFloat(cfg->GetFloat("MiddleStarsSize", 0.6f), 0.03f, 2.5f);
    fBiggestStars = ClampFloat(cfg->GetFloat("BiggestStarsSize", 1.2f), 0.03f, 2.5f);
    fBiggestStarsSpawnChance = 1.0f - 0.01f * ClampFloat(cfg->GetFloat("BiggestStarsChance", 20), 0.0f, 100.0f);
    
    // Keeps stars always the same
    srand(cfg->GetInt("StarsSeed", 0xBEEF));

    for(int side = 0; side < SSidesCount; ++side)
    {
        for(int i = 0; i < AMOUNT_OF_SIDESTARS; ++i)
        {
            StarCoorsX[side][i] = 95.0f * RandomIt(-1.0f, 1.0f);

            // Side=4 is when rendering stars directly ABOVE us
            if(side == SSide_Up) StarCoorsY[side][i] = 95.0f * RandomIt(-1.0f, 1.0f);
            else StarCoorsY[side][i] = 95.0f * RandomIt(-0.35f, 1.0f);

            // Smaller chances for a bigger star (this is more life-like)
            if(RandomIt(0.0f, 1.0f) > fBiggestStarsSpawnChance) StarSizes[side][i] = 0.8f * RandomIt(fSmallStars, fBiggestStars);
            else StarSizes[side][i] = 0.8f * RandomIt(fSmallStars, fMiddleStars);
        }
    }

    // Makes other rand() calls "more random"
    srand(time(NULL));
}