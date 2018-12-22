#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

static unsigned int CompileShader(unsigned int type, const std::string& source) {
	unsigned int id = glCreateShader(type);  //创建着色器
	const char* src = source.c_str();  //转化为字符串，我们需要指针
	//给GPU传入代码
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);
	
	//如果编译出错，输出错误信息(异常处理)
	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Failed to compile" <<
			(type == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< "shader!" << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
	unsigned int program = glCreateProgram();  
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader); //顶点着色器
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader); //顶点着色器
	
	//绑定着色器，将两个着色器绑定到GPU程序中
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);
	//当链接编译成功后不再需要着色器源代码
	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)  //在上面那条语句下开始
		std::cout << "ERROR!" << std::endl;
	std::cout << glGetString(GL_VERSION) << std::endl;  //版本

	float positions[6] = { //顶点
		-0.5f, -0.5f,
		 0.0f,  0.5f,
		 0.5f, -0.5f
	};

	unsigned int buffer;
	glGenBuffers(1, &buffer);  //申请GPU内存
	glBindBuffer(GL_ARRAY_BUFFER, buffer); //绑定内存
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW); //传入数据

	glEnableVertexAttribArray(0);  
	glVertexAttribPointer(0, 2, GL_FLOAT,GL_FALSE,2 * sizeof(float), 0); //确定顶点属性
	
	std::string vertexShader =
		"#version 330 core\n"
		"layout(location = 0) in vec4 position;\n"
		"void main()\n"
		"{"
		"	gl_Position = position;\n"
		"}";
	std::string fragmentShader =
		"#version 330 core\n"
		"layout(location = 0) out vec4 color;\n"
		"void main()\n"
		"{"
		"	color = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}";
	unsigned int shader = CreateShader(vertexShader, fragmentShader);
	glUseProgram(shader);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);
		
		glDrawArrays(GL_TRIANGLES, 0, 3); //绘制三角形
		
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}