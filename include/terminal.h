void Terminal_Print(char* string, unsigned int length);
void Terminal_PrintImportant(char* string, unsigned int length);

int Terminal_Initialize(void);
void Terminal_Clear(void);
void Terminal_Update(void);
void Terminal_PrintPrompt(void);
void Terminal_PrintWelcome(void);

void Terminal_back(void);

int Terminal_GetIsInitialized(void);
