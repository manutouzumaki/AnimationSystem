void *ReadFile(const char *fileName)
{
    void *data = 0;
    FILE *file = 0;
    fopen_s(&file, fileName, "rb");
    if(file)
    {
        fseek(file, 0, SEEK_END);
        size_t fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        data = malloc(fileSize + 1);
        fread_s(data, fileSize, fileSize, 1, file);
        char *lastByte = (char *)data + fileSize;
        *lastByte = 0;
    }
    fclose(file);
    return data;
}

u32 CompileShaderFromFile(const char *fileName, u32 type)
{
    u32 shader = glCreateShader(type);
    const char *shaderSource = (const char *)ReadFile(fileName);
    glShaderSource(shader, 1, &shaderSource, 0);
    glCompileShader(shader);
    
    i32 succes;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &succes);
    if(!succes)
    {
        printf("Error Compiling Shader\n");
        glGetShaderInfoLog(shader, 512, 0, infoLog);
        printf(infoLog);
    }
    free((void *)shaderSource);
    return shader; 
}

u32 CreateShaderProgram(u32 vertexShader, u32 fragmentShader)
{
    u32 program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    i32 succes;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &succes);
    if(!succes)
    {
        printf("Error linking Shader to Program\n");
        glGetProgramInfoLog(program, 512, 0, infoLog);
        printf(infoLog);
    }
    return program;
}
