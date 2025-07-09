bool (*CanSeeOutSideFromCurrArea)();
void (*RwRenderStateSet)(int, void*);
void (*InitSpriteBuffer)();
void (*FlushSpriteBuffer)();
bool (*CalcScreenCoors)(CVector*, CVector*, float*, float*, bool);
void (*RenderBufferedOneXLUSprite)(CVector, float, float, uint8_t, uint8_t, uint8_t, short, float, uint8_t);
void (*RenderBufferedOneXLUSprite_Rotate_Dimension)(CVector, float, float, uint8_t, uint8_t, uint8_t, short, float, float, uint8_t);
void (*RenderBufferedOneXLUSprite_Rotate_Aspect)(CVector, float, float, uint8_t, uint8_t, uint8_t, short, float, float, uint8_t);
void (*RenderBufferedOneXLUSprite_Rotate_2Colours)(CVector, float, float, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, float, float, float, float, uint8_t);
float (*GetATanOfXY)(float, float);
void* (*RwIm3DTransform)(RwIm3DVertex*, uint32_t, CMatrix*, uint32_t);
void (*RwIm3DRenderIndexedPrimitive)(uint32_t, uint16_t*, int);
void (*RwIm3DEnd)();

bool *SunBlockedByClouds;
float *Foggyness, *CloudCoverage, *ms_fAspectRatio, *ExtraSunnyness, *ms_cameraRoll, *CloudRotation, *Rainbow,
      *SunScreenX, *SunScreenY, *InterpolationValue;
void ***gpCoronaTexture, ***gpCloudTex;
uint8_t *ms_nGameClockHours, *ms_nGameClockMinutes, *ms_nGameClockSeconds;
int32_t *m_nCurrentLowCloudsRed, *m_nCurrentLowCloudsGreen, *m_nCurrentLowCloudsBlue,
        *m_nCurrentFluffyCloudsTopRed, *m_nCurrentFluffyCloudsTopGreen, *m_nCurrentFluffyCloudsTopBlue,
        *m_nCurrentFluffyCloudsBottomRed, *m_nCurrentFluffyCloudsBottomGreen, *m_nCurrentFluffyCloudsBottomBlue,
        *m_CurrentStoredValue;
uint32_t *IndividualRotation, *MoonSize, *m_snTimeInMilliseconds;
int16_t *NewWeatherType, *OldWeatherType;
IntVector2D *RsGlobal;
char* TheCamera;
CVector *CamPos, *m_VectorToSun;