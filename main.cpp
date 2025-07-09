#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>

#include <stdint.h>
#include <math.h>
#include <time.h>

#define IMPROVED_MOON
#define STARRY_SKIES

#define IMPROVED_MOON_HEIGHT    (50.0f)
#define STARS_STARTHOUR         22
#define START_LASTHOUR          5



uintptr_t pGTAVC;
void *hGTAVC;
bool hasJPatch15;

#include "SimpleGTA.h"
#include "vars.inl"
#ifdef STARRY_SKIES
    #include "starryskies.inl"
#else
    float StarCoorsX[9] = { 0.0f, 0.05f, 0.13f, 0.4f, 0.7f, 0.6f, 0.27f, 0.55f, 0.75f };
    float StarCoorsY[9] = { 0.0f, 0.45f, 0.9f, 1.0f, 0.85f, 0.52f, 0.48f, 0.35f, 0.2f };
    float StarSizes[9] = { 1.0f, 1.4f, 0.9f, 1.0f, 0.6f, 1.5f, 1.3f, 1.0f, 0.8f };
#endif



MYMOD(net.rusjj.viceskies, ViceSkies, 1.3, RusJJ)
NEEDGAME(com.rockstargames.gtavc)

float LowCloudsX[12] = { 1.0f,  0.7f,  0.0f, -0.7f, -1.0f, -0.7f, 0.0f, 0.7f, 0.8f, -0.8f,  0.4f, -0.4f };
float LowCloudsY[12] = { 0.0f, -0.7f, -1.0f, -0.7f,  0.0f,  0.7f, 1.0f, 0.7f, 0.4f,  0.4f, -0.8f, -0.8f };
float LowCloudsZ[12] = { 0.0f,  1.0f,  0.5f,  0.0f,  1.0f,  0.3f, 0.9f, 0.4f, 1.3f,  1.4f,  1.2f,  1.7f };

uint8_t BowRed[6]   = { 30, 30, 30, 10,  0, 15 };
uint8_t BowGreen[6] = {  0, 15, 30, 30,  0,  0 };
uint8_t BowBlue[6]  = {  0,  0,  0, 10, 30, 30 };

float CoorsOffsetX[37] = {
    0.0f, 60.0f, 72.0f, 48.0f, 21.0f, 12.0f,
    9.0f, -3.0f, -8.4f, -18.0f, -15.0f, -36.0f,
    -40.0f, -48.0f, -60.0f, -24.0f, 100.0f, 100.0f,
    100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f,
    100.0f, 100.0f, -30.0f, -20.0f, 10.0f, 30.0f,
    0.0f, -100.0f, -100.0f, -100.0f, -100.0f, -100.0f, -100.0f
};
float CoorsOffsetY[37] = {
    100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f,
    100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f,
    100.0f, 100.0f, 100.0f, 100.0f, -30.0f, 10.0f,
    -25.0f, -5.0f, 28.0f, -10.0f, 10.0f, 0.0f,
    15.0f, 40.0f, -100.0f, -100.0f, -100.0f, -100.0f,
    -100.0f, -40.0f, -20.0f, 0.0f, 10.0f, 30.0f, 35.0f
};
float CoorsOffsetZ[37] = {
    2.0f, 1.0f, 0.0f, 0.3f, 0.7f, 1.4f,
    1.7f, 0.24f, 0.7f, 1.3f, 1.6f, 1.0f,
    1.2f, 0.3f, 0.7f, 1.4f, 0.0f, 0.1f,
    0.5f, 0.4f, 0.55f, 0.75f, 1.0f, 1.4f,
    1.7f, 2.0f, 2.0f, 2.3f, 1.9f, 2.4f,
    2.0f, 2.0f, 1.5f, 1.2f, 1.7f, 1.5f, 2.1f
};

RwIm3DVertex Skies_TempBufferRenderVertices[32];
uint16_t pShootingStarIndices[] = { 0, 1 };

CVector MoonVector;
inline float SQR(float v) { return v*v; }
DECL_HOOKv(RenderClouds)
{
    float szx, szy;
    RwV3d screenpos;
    RwV3d worldpos;
    
    if(!CanSeeOutSideFromCurrArea()) return;
    
    RwRenderStateSet(8, (void*)0);
    RwRenderStateSet(6, (void*)0);
    RwRenderStateSet(12, (void*)1);
    RwRenderStateSet(10, (void*)2);
    RwRenderStateSet(11, (void*)2);
    InitSpriteBuffer();
    
    *SunBlockedByClouds = false;
    float coverage = fmaxf(*Foggyness, *CloudCoverage);
    float decoverage = 1.0f - coverage;
    
    // Stars
    if(coverage < 1 && (*ms_nGameClockHours >= STARS_STARTHOUR || *ms_nGameClockHours <= START_LASTHOUR))
    {
        float brightness = 255.0f * decoverage;
        if(*ms_nGameClockHours == STARS_STARTHOUR) brightness *= (0.0166667f * *ms_nGameClockMinutes);
        else if(*ms_nGameClockHours == START_LASTHOUR) brightness *= (0.0166667f * (60 - *ms_nGameClockMinutes));
        
        RwRenderStateSet(1, *(gpCoronaTexture[0]));
      #ifdef STARRY_SKIES
        StarrySkies_Patch(brightness);
      #else
        for(int i = 0; i < 11; ++i)
        {
            RwV3d pos = { 100.0f, 0.0f, 10.0f };
            if(i >= 9) pos.x = -pos.x;
            worldpos = pos + *CamPos;
            worldpos.y -= 90.0f * StarCoorsX[i % 9];
			worldpos.z += 80.0f * StarCoorsY[i % 9];
            
            if(CalcScreenCoors(&worldpos, &screenpos, &szx, &szy, false))
            {
                float sz = 0.8f * StarSizes[i % 9];
                if(!hasJPatch15) szx /= *ms_fAspectRatio;
                RenderBufferedOneXLUSprite(screenpos, szx * sz, szy * sz, brightness, brightness, brightness, 255, 1.0f / screenpos.z, 255);
            }
        }
      #endif
        FlushSpriteBuffer();
    }
    
    // Moon
    float minute = 60.0f * *ms_nGameClockHours + *ms_nGameClockMinutes;// + 0.0166667f * *ms_nGameClockSeconds; // useless part
  #ifdef IMPROVED_MOON
    int moonfadeout;
    float smoothBrightnessAdjust = 1.9f;
    if(minute > 1100)
    {
        moonfadeout = (int)(fabsf(minute - 1100.0f) / smoothBrightnessAdjust);
    }
    else if(minute < 240)
    {
        moonfadeout = 180;
    }
    else
    {
        moonfadeout = (int)(180.0f - fabsf(minute - 240.0f) * smoothBrightnessAdjust);
    }
    
    if (moonfadeout > 0 && moonfadeout < 340)
    {
        CVector& vecsun = m_VectorToSun[*m_CurrentStoredValue];
        MoonVector = { -vecsun.x, -vecsun.y, -(IMPROVED_MOON_HEIGHT / 150.0f) * vecsun.z }; // normalized vector (important for DotProd)
        RwV3d pos = { 150.0f * MoonVector.x, 150.0f * MoonVector.y, 150.0f * MoonVector.z };
  #else
    int moonfadeout = (int)(fabsf(minute - 180.0f));
    if(moonfadeout < 180)
    {
        RwV3d pos = { 0.0f, -100.0f, 15.0f };
  #endif
        worldpos = pos + *CamPos;
        if(CalcScreenCoors(&worldpos, &screenpos, &szx, &szy, false))
        {
            RwRenderStateSet(1, *(gpCoronaTexture[2]));
          #ifdef IMPROVED_MOON
            float sz = *MoonSize * 2.7f + 4.0f;
            int brightness = decoverage * moonfadeout;
          #else
            float sz = *MoonSize * 2.0f + 4.0f;
            int brightness = decoverage * (180 - moonfadeout);
          #endif
            if(!hasJPatch15) szx /= *ms_fAspectRatio;
            RenderBufferedOneXLUSprite(screenpos, szx * sz, szy * sz, brightness, brightness, brightness, 255, 1.0f / screenpos.z, 255);
            FlushSpriteBuffer();
        }
    }
    
    // Low Clouds
    float lowcintens = 1.0f - fmaxf(coverage, *ExtraSunnyness);
    int r = *m_nCurrentLowCloudsRed   * lowcintens;
    int g = *m_nCurrentLowCloudsGreen * lowcintens;
    int b = *m_nCurrentLowCloudsBlue  * lowcintens;
    for(int cloudtype = 0; cloudtype < 3; ++cloudtype)
    {
        RwRenderStateSet(1, *(gpCloudTex[cloudtype]));
        for(int i = cloudtype; i < 12; i += 3)
        {
            RwV3d pos = { 800.0f * LowCloudsX[i], 800.0f * LowCloudsY[i], 60.0f * LowCloudsZ[i] };
            worldpos.x = CamPos->x + pos.x;
            worldpos.y = CamPos->y + pos.y;
            worldpos.z = 40.0f + pos.z;
            if(CalcScreenCoors(&worldpos, &screenpos, &szx, &szy, false))
            {
                if(!hasJPatch15) szx /= *ms_fAspectRatio;
                RenderBufferedOneXLUSprite_Rotate_Dimension(screenpos, szx * 320.0f, szy * 40.0f, r, g, b, 255, 1.0f / screenpos.z, *ms_cameraRoll, 255);
            }
        }
        FlushSpriteBuffer();
    }
    
    // Fluffy Clouds
    if(coverage < 1)
    {
        float ARdiff = (3.0f * *ms_fAspectRatio) / 4.0f; // is it necessary? gonna check later!
        float distLimit = ((3.0f * (float)(RsGlobal->x)) / 4.0f) * ARdiff;
        float sundistBlocked = ((float)(RsGlobal->x) / 10.0f) / ARdiff;
        float sundistHilit = ((float)(RsGlobal->x) / 3.0) / ARdiff;
        
        static bool bCloudOnScreen[37];
        static float fSunDist[37];
        static float fCloudHighlight[37];
        
        int fluffyalpha = 160 * decoverage;
        float rot_sin = sinf(*CloudRotation);
        float rot_cos = cosf(*CloudRotation);

        float rotationValue = ((2.0f * M_PI) * (uint16_t)*IndividualRotation) / 65536.0f + *ms_cameraRoll;
        
        RwRenderStateSet(10, (void*)5);
        RwRenderStateSet(11, (void*)6);
        RwRenderStateSet(1, *(gpCloudTex[4]));
        for(int i = 0; i < 37; ++i)
        {
            RwV3d pos = { 2.0f*CoorsOffsetX[i], 2.0f*CoorsOffsetY[i], 40.0f*CoorsOffsetZ[i] + 40.0f };
            worldpos.x = pos.x*rot_cos + pos.y*rot_sin + CamPos->x;
            worldpos.y = pos.x*rot_sin + pos.y*rot_cos + CamPos->y;
            worldpos.z = pos.z;
            
            if(CalcScreenCoors(&worldpos, &screenpos, &szx, &szy, false))
            {
                fSunDist[i] = sqrtf(SQR(screenpos.x - *SunScreenX) + SQR(screenpos.y - *SunScreenY));
                fCloudHighlight[i] = decoverage * (1.0f - fSunDist[i] / distLimit);
                
                int tr = *m_nCurrentFluffyCloudsTopRed;
                int tg = *m_nCurrentFluffyCloudsTopGreen;
                int tb = *m_nCurrentFluffyCloudsTopBlue;
                
                int br = *m_nCurrentFluffyCloudsBottomRed;
                int bg = *m_nCurrentFluffyCloudsBottomGreen;
                int bb = *m_nCurrentFluffyCloudsBottomBlue;
                
                if(fSunDist[i] < distLimit)
                {
                    tr = tr * (1.0f - fCloudHighlight[i]) + 235 * fCloudHighlight[i];
                    tg = tg * (1.0f - fCloudHighlight[i]) + 190 * fCloudHighlight[i];
                    tb = tb * (1.0f - fCloudHighlight[i]) + 190 * fCloudHighlight[i];
                    br = br * (1.0f - fCloudHighlight[i]) + 235 * fCloudHighlight[i];
                    bg = bg * (1.0f - fCloudHighlight[i]) + 190 * fCloudHighlight[i];
                    bb = bb * (1.0f - fCloudHighlight[i]) + 190 * fCloudHighlight[i];
                    if(fSunDist[i] < sundistBlocked) *SunBlockedByClouds = (fluffyalpha > 50);
                }
                else
                {
                    fCloudHighlight[i] = 0.0f;
                }
                if(!hasJPatch15) szx /= *ms_fAspectRatio;
                RenderBufferedOneXLUSprite_Rotate_2Colours(screenpos, szx * 55.0f, szy * 55.0f, tr, tg, tb, br, bg, bb, 0.0f, -1.0f, 1.0f / screenpos.z, rotationValue, fluffyalpha);
                bCloudOnScreen[i] = true;
            }
            else
            {
                bCloudOnScreen[i] = false;
            }
        }
        FlushSpriteBuffer();
        
        RwRenderStateSet(10, (void*)2);
        RwRenderStateSet(11, (void*)2);
        RwRenderStateSet(1, *(gpCloudTex[3]));
        for(int i = 0; i < 37; ++i)
        {
            RwV3d pos = { 2.0f * CoorsOffsetX[i], 2.0f * CoorsOffsetY[i], 40.0f * CoorsOffsetZ[i] + 40.0f };
            worldpos.x = pos.x*rot_cos + pos.y*rot_sin + CamPos->x;
            worldpos.y = pos.x*rot_sin + pos.y*rot_cos + CamPos->y;
            worldpos.z = pos.z;
            
            if(bCloudOnScreen[i] && CalcScreenCoors(&worldpos, &screenpos, &szx, &szy, false) && fSunDist[i] < sundistHilit)
            {
                if(!hasJPatch15) szx /= *ms_fAspectRatio;
                RenderBufferedOneXLUSprite_Rotate_Aspect(screenpos, szx * 30.0f, szy * 30.0f, 200 * fCloudHighlight[i], 0, 0, 255, 1.0f / screenpos.z,
                                                         1.7f - GetATanOfXY(screenpos.x - *SunScreenX, screenpos.y - *SunScreenY) + *ms_cameraRoll, 255);
            }
        }
        FlushSpriteBuffer();
    }
    
    // Rainbow
    if(*Rainbow != 0)
    {
        RwRenderStateSet(1, *(gpCoronaTexture[0]));
        for(int i = 0; i < 6; ++i)
        {
            RwV3d pos = { i * 1.5f, 100.0f, 5.0f };
            worldpos = pos + *CamPos;
            if(CalcScreenCoors(&worldpos, &screenpos, &szx, &szy, false))
            {
                if(!hasJPatch15) szx /= *ms_fAspectRatio;
                RenderBufferedOneXLUSprite(screenpos, 2.0f * szx, 50.0 * szy, BowRed[i] * *Rainbow, BowGreen[i] * *Rainbow, BowBlue[i] * *Rainbow, 255, 1.0f / screenpos.z, 255);
            }
        }
        FlushSpriteBuffer();
    }

    // Falling Star (backported from San Andreas)
    if ( (*NewWeatherType == 0 || *NewWeatherType == 4) )
    {
        uint32_t v36 = (*m_snTimeInMilliseconds & 0x1FFF);
        if (v36 < 800)
        {
            float v43 = (400 - v36) + (400 - v36);
            Skies_TempBufferRenderVertices[0].color = 0xE1FFFFFF;
            Skies_TempBufferRenderVertices[1].color = 0x00FFFFFF;

            uint32_t v37 = (*m_snTimeInMilliseconds >> 13) & 0x3F;
            CVector starScale{ 0.1f * (v37 % 7 - 3), 0.1f * ((*m_snTimeInMilliseconds >> 13) - 4), 1.0f };
            starScale.Normalise();
            CVector starDir{ (float)(v37 % 9 - 5), (float)(v37 % 10 - 5), 0.1f };
            starDir.Normalise();

            worldpos = *CamPos + starDir * 1000.0f;

            Skies_TempBufferRenderVertices[0].pos = worldpos + starScale * v43;
            Skies_TempBufferRenderVertices[1].pos = worldpos + starScale * (v43 + 50.0f);

            RwRenderStateSet(1, (void*)0); // FIX_BUGS
            RwRenderStateSet(10, (void*)5);
            RwRenderStateSet(11, (void*)6);

            if (RwIm3DTransform(Skies_TempBufferRenderVertices, 2, NULL, 0x10 | 0x8))
            {
                RwIm3DRenderIndexedPrimitive(2, pShootingStarIndices, 2);
                RwIm3DEnd();
            }
        }
    }
    
    RwRenderStateSet(8, (void*)1);
    RwRenderStateSet(6, (void*)1);
    RwRenderStateSet(12, (void*)0);
    RwRenderStateSet(10, (void*)5);
    RwRenderStateSet(11, (void*)6);
}

#ifdef IMPROVED_MOON
uintptr_t FireSniper_BackTo;
#define DotProduct(v1, v2) (v1.z * v2.z + v1.y * v2.y + v1.x * v2.x)
extern "C" void FireSniper_Patch(CVector& m_vecFront)
{
    float dotprod = DotProduct(m_vecFront, MoonVector);
    if(dotprod > 0.985f) *MoonSize = (*MoonSize + 1) % 8; // 0.997 -> 0.985
}
__attribute__((optnone)) __attribute__((naked)) void FireSniper_Inject(void)
{
  #ifdef AML32
    asm volatile(
        "MOV             R0, R9\n"
        "BL              FireSniper_Patch\n");
    asm volatile(
        "MOV             PC, %0\n"
    :: "r" (FireSniper_BackTo));
  #else // AML64
    asm volatile(
        "ADD             X0, SP, #0x10\n"
        "STR             S1, [SP, #-16]!\n" // PUSH {S1}
        "BL              FireSniper_Patch\n");
    asm volatile(
        "MOV             X0, %0\n"
    :: "r" (FireSniper_BackTo));
    asm volatile(
        "LDR             S1, [SP], #16\n" // POP {S1}
        "BR              X0\n");
  #endif
}
#endif

uintptr_t WeatherUpdate_BackTo;
extern "C" void WeatherUpdate_Patch()
{
    if(*NewWeatherType != 0 && *NewWeatherType != 4 && *OldWeatherType != 0 && *OldWeatherType != 4)
    {
        *CloudCoverage += *InterpolationValue;
    }
}
__attribute__((optnone)) __attribute__((naked)) void WeatherUpdate_Inject(void)
{
  #ifdef AML32
    asm volatile(
        "PUSH            {R1-R4}\n"
        "BL              WeatherUpdate_Patch\n"
        "POP             {R1-R4}\n");
    asm volatile(
        "MOV             PC, %0\n"
    :: "r" (WeatherUpdate_BackTo));
  #else // AML64
    asm volatile(
        "STR             X8, [SP, #-16]!\n" // PUSH {X8}
        "STR             X9, [SP, #-16]!\n" // PUSH {X9}
        "BL              WeatherUpdate_Patch\n");
    asm volatile(
        "MOV             X0, %0\n"
    :: "r" (WeatherUpdate_BackTo));
    asm volatile(
        "LDR             X9, [SP], #16\n" // POP {X9}
        "LDR             X8, [SP], #16\n" // POP {X8}
        "BR              X0\n");
  #endif
}

extern "C" void OnModLoad()
{
    logger->SetTag("Vice Skies");
    pGTAVC = aml->GetLib("libGTAVC.so");
    hGTAVC = aml->GetLibHandle("libGTAVC.so");
    hasJPatch15 = aml->HasModOfVersion("net.rusjj.jpatch", "1.5");
    
    if(!pGTAVC || !hGTAVC)
    {
        logger->Error("This mod only for Vice City. Stopping.");
        return;
    }

    logger->Info("Warming up the code...");
    
    SET_TO(CanSeeOutSideFromCurrArea,  aml->GetSym(hGTAVC, "_ZN5CGame25CanSeeOutSideFromCurrAreaEv"));
    SET_TO(RwRenderStateSet,           aml->GetSym(hGTAVC, "_Z16RwRenderStateSet13RwRenderStatePv"));
    SET_TO(InitSpriteBuffer,           aml->GetSym(hGTAVC, "_ZN7CSprite16InitSpriteBufferEv"));
    SET_TO(FlushSpriteBuffer,          aml->GetSym(hGTAVC, "_ZN7CSprite17FlushSpriteBufferEv"));
    SET_TO(CalcScreenCoors,            aml->GetSym(hGTAVC, "_ZN7CSprite15CalcScreenCoorsERK5RwV3dPS0_PfS4_b"));
    SET_TO(RenderBufferedOneXLUSprite, aml->GetSym(hGTAVC, "_ZN7CSprite26RenderBufferedOneXLUSpriteEfffffhhhsfh"));
    SET_TO(RenderBufferedOneXLUSprite_Rotate_Dimension, aml->GetSym(hGTAVC, "_ZN7CSprite43RenderBufferedOneXLUSprite_Rotate_DimensionEfffffhhhsffh"));
    SET_TO(RenderBufferedOneXLUSprite_Rotate_Aspect,    aml->GetSym(hGTAVC, "_ZN7CSprite40RenderBufferedOneXLUSprite_Rotate_AspectEfffffhhhsffh"));
    SET_TO(RenderBufferedOneXLUSprite_Rotate_2Colours,  aml->GetSym(hGTAVC, "_ZN7CSprite42RenderBufferedOneXLUSprite_Rotate_2ColoursEfffffhhhhhhffffh"));
    SET_TO(GetATanOfXY,                aml->GetSym(hGTAVC, "_ZN8CGeneral11GetATanOfXYEff"));
    SET_TO(RwIm3DTransform,            aml->GetSym(hGTAVC, "_Z15RwIm3DTransformP18RxObjSpace3DVertexjP11RwMatrixTagj"));
    SET_TO(RwIm3DRenderIndexedPrimitive,aml->GetSym(hGTAVC, "_Z28RwIm3DRenderIndexedPrimitive15RwPrimitiveTypePti"));
    SET_TO(RwIm3DEnd,                  aml->GetSym(hGTAVC, "_Z9RwIm3DEndv"));
    
    SET_TO(SunBlockedByClouds,         aml->GetSym(hGTAVC, "_ZN8CCoronas18SunBlockedByCloudsE"));
    SET_TO(Foggyness,                  aml->GetSym(hGTAVC, "_ZN8CWeather9FoggynessE"));
    SET_TO(CloudCoverage,              aml->GetSym(hGTAVC, "_ZN8CWeather13CloudCoverageE"));
    SET_TO(ms_fAspectRatio,            aml->GetSym(hGTAVC, "_ZN5CDraw15ms_fAspectRatioE"));
    SET_TO(ExtraSunnyness,             aml->GetSym(hGTAVC, "_ZN8CWeather14ExtraSunnynessE"));
    SET_TO(ms_cameraRoll,              aml->GetSym(hGTAVC, "_ZN7CClouds13ms_cameraRollE"));
    SET_TO(CloudRotation,              aml->GetSym(hGTAVC, "_ZN7CClouds13CloudRotationE"));
    SET_TO(Rainbow,                    aml->GetSym(hGTAVC, "_ZN8CWeather7RainbowE"));
    SET_TO(SunScreenX,                 aml->GetSym(hGTAVC, "_ZN8CCoronas10SunScreenXE"));
    SET_TO(SunScreenY,                 aml->GetSym(hGTAVC, "_ZN8CCoronas10SunScreenYE"));
    SET_TO(InterpolationValue,         aml->GetSym(hGTAVC, "_ZN8CWeather18InterpolationValueE"));
    SET_TO(gpCoronaTexture,            aml->GetSym(hGTAVC, "gpCoronaTexture"));
    SET_TO(gpCloudTex,                 aml->GetSym(hGTAVC, "gpCloudTex"));
    SET_TO(ms_nGameClockHours,         aml->GetSym(hGTAVC, "_ZN6CClock18ms_nGameClockHoursE"));
    SET_TO(ms_nGameClockMinutes,       aml->GetSym(hGTAVC, "_ZN6CClock20ms_nGameClockMinutesE"));
    SET_TO(ms_nGameClockSeconds,       aml->GetSym(hGTAVC, "_ZN6CClock20ms_nGameClockSecondsE"));
    SET_TO(m_nCurrentLowCloudsRed,     aml->GetSym(hGTAVC, "_ZN10CTimeCycle22m_nCurrentLowCloudsRedE"));
    SET_TO(m_nCurrentLowCloudsGreen,   aml->GetSym(hGTAVC, "_ZN10CTimeCycle24m_nCurrentLowCloudsGreenE"));
    SET_TO(m_nCurrentLowCloudsBlue,    aml->GetSym(hGTAVC, "_ZN10CTimeCycle23m_nCurrentLowCloudsBlueE"));
    SET_TO(m_nCurrentFluffyCloudsTopRed,      aml->GetSym(hGTAVC, "_ZN10CTimeCycle28m_nCurrentFluffyCloudsTopRedE"));
    SET_TO(m_nCurrentFluffyCloudsTopGreen,    aml->GetSym(hGTAVC, "_ZN10CTimeCycle30m_nCurrentFluffyCloudsTopGreenE"));
    SET_TO(m_nCurrentFluffyCloudsTopBlue,     aml->GetSym(hGTAVC, "_ZN10CTimeCycle29m_nCurrentFluffyCloudsTopBlueE"));
    SET_TO(m_nCurrentFluffyCloudsBottomRed,   aml->GetSym(hGTAVC, "_ZN10CTimeCycle31m_nCurrentFluffyCloudsBottomRedE"));
    SET_TO(m_nCurrentFluffyCloudsBottomGreen, aml->GetSym(hGTAVC, "_ZN10CTimeCycle33m_nCurrentFluffyCloudsBottomGreenE"));
    SET_TO(m_nCurrentFluffyCloudsBottomBlue,  aml->GetSym(hGTAVC, "_ZN10CTimeCycle32m_nCurrentFluffyCloudsBottomBlueE"));
    SET_TO(m_CurrentStoredValue,       aml->GetSym(hGTAVC, "_ZN10CTimeCycle20m_CurrentStoredValueE"));
    SET_TO(IndividualRotation,         aml->GetSym(hGTAVC, "_ZN7CClouds18IndividualRotationE"));
    SET_TO(MoonSize,                   aml->GetSym(hGTAVC, "_ZN8CCoronas8MoonSizeE"));
    SET_TO(m_snTimeInMilliseconds,     aml->GetSym(hGTAVC, "_ZN6CTimer22m_snTimeInMillisecondsE"));
    SET_TO(NewWeatherType,             aml->GetSym(hGTAVC, "_ZN8CWeather14NewWeatherTypeE"));
    SET_TO(OldWeatherType,             aml->GetSym(hGTAVC, "_ZN8CWeather14OldWeatherTypeE"));
    SET_TO(RsGlobal,                   aml->GetSym(hGTAVC, "RsGlobal"));
    SET_TO(TheCamera,                  aml->GetSym(hGTAVC, "TheCamera"));
    SET_TO(m_VectorToSun,              aml->GetSym(hGTAVC, "_ZN10CTimeCycle13m_VectorToSunE"));
    CamPos = (CVector*)(TheCamera + 0x30); // both in 1.09 and 1.12
    
    HOOKBL(RenderClouds, pGTAVC + BYBIT(0x14EA6E + 0x1, 0x1FA750)); // RenderScene
    HOOKBL(RenderClouds, pGTAVC + BYBIT(0x14D8DC + 0x1, 0x1F99DC)); // NewTileRendererCB
    
  #ifdef IMPROVED_MOON
    FireSniper_BackTo = pGTAVC + BYBIT(0x26B056 + 0x1, 0x363758);
    aml->Redirect(pGTAVC + BYBIT(0x26B036 + 0x1, 0x36373C), (uintptr_t)FireSniper_Inject);

    // A fix for moon disappearing on Sunny->Extrasunny weather !!!
    WeatherUpdate_BackTo = pGTAVC + BYBIT(0x211F88 + 0x1, 0x2F8708);
    aml->Redirect(pGTAVC + BYBIT(0x211F7A + 0x1, 0x2F86F4), (uintptr_t)WeatherUpdate_Inject);
  #endif

  #ifdef STARRY_SKIES
    InitializeThoseStars();
  #endif
}

Config cfgLocal("ViceSkies"); Config* cfg = &cfgLocal;