#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

class HelloTriangleApplication{

private:
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;

	const int WIDTH = 800;
	const int HEIGHT = 600;

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };

public:
	HelloTriangleApplication() {};
	~HelloTriangleApplication() {};

	void run();

private:
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	bool checkValidationLayerSupport();

	std::vector<const char*> getRequiredExtensions();

	void initWindow();

	void initVulkan();

	void setupDebugMessenger();

	void createInstance();

	void mainLoop();

	void cleanup();

};

