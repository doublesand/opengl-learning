#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define ASSERT(x) if (!(x)) __debugbreak(); 
#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))
	


static void GLClearError() {
	while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, int line) {
	while (GLenum error = glGetError()) {
		std::cout << "[OpenGL Error] (" << error << "): " <<function<<
			" "<< file << ":" << line << std::endl;
		return false;
	}
	return true; 
}

struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath) {
	std::ifstream stream(filepath);
	
	enum class ShaderType {
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};
	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;
	while (getline(stream, line)) {
		if (line.find("#shader") != std::string::npos) {
			if (line.find("vertex") != std::string::npos)
				type = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)
				type = ShaderType::FRAGMENT;
		}
		else {
			ss[(int)type] << line << "\n";
		}
	}
	return { ss[0].str(), ss[1].str() };
}


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

	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)  //在上面那条语句下开始
		std::cout << "ERROR!" << std::endl;
	std::cout << glGetString(GL_VERSION) << std::endl;  //版本

	float positions[] = { //顶点
		-0.5f, -0.5f,  //0
		 0.5f, -0.5f,  //1
		 0.5f,  0.5f,  //2
		-0.5f,  0.5f   //3
	};

	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	unsigned int buffer;
	glGenBuffers(1, &buffer);  //申请GPU内存
	glBindBuffer(GL_ARRAY_BUFFER, buffer); //绑定内存
	glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW); //传入数据
	
	//开启并声明顶点属性，以至于着色器可以访问
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0); //确定正方形的顶点属性

	unsigned int ibo;
	glGenBuffers(1, &ibo);  //申请GPU内存
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); //绑定内存
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW); //传入数据

	ShaderProgramSource source = ParseShader("Basic.shader");
	unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
	glUseProgram(shader);

	//开启着色器之后使用uniform给着色器传变量
	int location = glGetUniformLocation(shader, "u_Color");
	ASSERT(location != -1); //如果是-1说明着色器没有这个变量
	
	float r = 0.0f; //红色分量渐变色
	float increment = 0.05f;  //颜色步长
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		glUniform4f(location, r, 0.6f, 0.1f, 1.0f);

		//第一次传入的索引可以找到数组里的顶点并绘制，根据绘制的类型找到顶点数据

		GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

		if (r > 1.0f)
			increment = -0.05f;
		else if (r < 0.0f)
			increment = 0.05f;
		r += increment;

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glDeleteProgram(shader);

	glfwTerminate();
	return 0;
}