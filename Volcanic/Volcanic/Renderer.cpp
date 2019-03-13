#include "Renderer.h"
#include <cstdlib>
#include <assert.h>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#endif

Renderer::Renderer()
{
	//Initalise Vulkan Instance and Device
	SetupDebug();
	InitInstance();
	InitDebug();
	InitDevice();
}


Renderer::~Renderer()
{
	//DeInitalise Device and Vulkan Instance 
	DeInitDevice();
	DeInitDebug();
	DeInitInstance();
}


void Renderer::InitInstance()
{
	//Define the Application info such as name & version
	VkApplicationInfo Application_Info {};
	Application_Info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	Application_Info.apiVersion = VK_MAKE_VERSION(1, 0, 3);
	Application_Info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	Application_Info.pApplicationName = "Volcanic Engine";

	//Attatch the application info to the Vulkan instance creation info
	VkInstanceCreateInfo InstanceCreateInfo {};
	InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	InstanceCreateInfo.pApplicationInfo = &Application_Info;
	InstanceCreateInfo.enabledLayerCount = _instance_layers.size();
	InstanceCreateInfo.ppEnabledLayerNames = _instance_layers.data();
	InstanceCreateInfo.enabledExtensionCount = _instance_extensions.size();
	InstanceCreateInfo.ppEnabledExtensionNames = _instance_extensions.data();
	InstanceCreateInfo.pNext = &debug_callback_create_info;


	//Create Vulkan instance
	auto err = vkCreateInstance(&InstanceCreateInfo, nullptr, &_instance);
	//Check that the Vulkan instance was created successfully
	if (err != VK_SUCCESS) 
	{
		assert(0 && "Vulkan ERROR: Create instance FAILED.");
		std::exit(-1);
	}
}


void Renderer::DeInitInstance()
{
	//Destroys the Vulkan Instance
	vkDestroyInstance(_instance, nullptr);
	_instance = nullptr;
}


void Renderer::InitDevice()
{
	uint32_t gpu_count = 0;

	//First call to EnumPhysicalDevices gets the number of physical GPUs
	vkEnumeratePhysicalDevices(_instance, &gpu_count, nullptr);
	std::vector<VkPhysicalDevice> gpu_list(gpu_count);

	//Second call populates the gpu_list with all found GPUs
	vkEnumeratePhysicalDevices(_instance, &gpu_count, gpu_list.data());

	//Sets the Current GPU to be the first GPU in the list(Check for most powerful GPU in future)
	_gpu = gpu_list[0];

	//Gets the GPU device properties of the current GPU such as Name and Technical limitations
	vkGetPhysicalDeviceProperties(_gpu, &_gpu_properties);




	uint32_t family_count = 0;
	
	//Fist call to GetQueueFamilyProperties gets the number of Family types available on the current GPU
	vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &family_count, nullptr);
	std::vector<VkQueueFamilyProperties> family_property_list(family_count);

	//Second call populates the family_property_list with all the Family types found
	vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &family_count, family_property_list.data());


	//Search through all Families to find all families with support for graphics
	bool found = false;
	for (uint32_t i = 0; i < family_count; i++)
	{
		if (family_property_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			found = true;
			_graphics_family_index = i;
		}
	}

	//Check that atleast one Graphics capable family was found
	if (!found)
	{
		assert(0 && "Vulkan ERROR: Queue family supporting graphics NOT found.");
		std::exit(-1);
	}




	uint32_t layer_count = 0;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	std::vector<VkLayerProperties> layer_property_list(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, layer_property_list.data());

	std::cout << "Instance layers: \n";
	for (auto &i : layer_property_list)
	{
		std::cout << "	" << i.layerName << "\t\t | " << i.description << std::endl;
	}
	std::cout << std::endl;




	uint32_t device_layer_count = 0;
	vkEnumerateDeviceLayerProperties(_gpu, &device_layer_count, nullptr);
	std::vector<VkLayerProperties> device_layer_property_list(device_layer_count);
	vkEnumerateDeviceLayerProperties(_gpu, &device_layer_count, device_layer_property_list.data());

	std::cout << "Device layers: \n";
	for (auto &i : device_layer_property_list)
	{
		std::cout << "	" << i.layerName << "\t\t | " << i.description << std::endl;
	}
	std::cout << std::endl;




	//Define the Queue creation info
	float queue_priorities[] = { 1.0f };
	VkDeviceQueueCreateInfo device_queue_create_info {};
	device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	device_queue_create_info.queueFamilyIndex = _graphics_family_index;
	device_queue_create_info.queueCount = 1;
	device_queue_create_info.pQueuePriorities = queue_priorities;

	//Attatch the Queue creation info to the device creation info
	VkDeviceCreateInfo device_create_info {};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pQueueCreateInfos = &device_queue_create_info;
	device_create_info.enabledLayerCount = _device_layers.size();
	device_create_info.ppEnabledLayerNames = _device_layers.data();
	device_create_info.enabledExtensionCount = _device_extensions.size();
	device_create_info.ppEnabledExtensionNames = _device_extensions.data();

	//Create the Device (Physical GPU)
	auto err = vkCreateDevice(_gpu, &device_create_info, nullptr, &_device);
	//Check that the device was created Successfully
	if (err != VK_SUCCESS)
	{
		assert(0 && "Vulkan ERROR: Device creation FAILED.");
		std::exit(-1);
	}
}


void Renderer::DeInitDevice()
{
	//Destroys the Physical Device
	vkDestroyDevice(_device, nullptr);
	_device = nullptr;
}


VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT obj_type, uint64_t src_obj, size_t location, int32_t msg_code, const char* layer_prefix, const char* msg, void* user_data)
{
	std::ostringstream stream;
	stream << "VKDBG: " << std::endl;
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		stream << "Info: ";
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		stream << "Warning: ";
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		stream << "Performance: ";
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		stream << "Error: ";
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		stream << "Debug: ";

	stream << "@[" << layer_prefix << "]: ";

	stream << msg << std::endl;

	std::cout << stream.str();


#ifdef _WIN32
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		MessageBox(NULL, stream.str().c_str(), "Vulkan Error!", 0);
	}
#endif

	return false;
}


void Renderer::SetupDebug()
{

	debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debug_callback_create_info.pfnCallback = VulkanDebugCallback;
	debug_callback_create_info.flags =
		VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_DEBUG_BIT_EXT |
		0;

	//_instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");
	//_instance_layers.push_back("VK_LAYER_LUNARG_threading");
	_instance_layers.push_back("VK_LAYER_LUNARG_draw_state");
	_instance_layers.push_back("VK_LAYER_LUNARG_image");
	_instance_layers.push_back("VK_LAYER_LUNARG_mem_tracker");
	_instance_layers.push_back("VK_LAYER_LUNARG_object_tracker");
	_instance_layers.push_back("VK_LAYER_LUNARG_param_tracker");

	//_device_layers.push_back("VK_LAYER_LUNARG_threading");
	_device_layers.push_back("VK_LAYER_LUNARG_draw_state");
	_device_layers.push_back("VK_LAYER_LUNARG_image");
	_device_layers.push_back("VK_LAYER_LUNARG_mem_tracker");
	_device_layers.push_back("VK_LAYER_LUNARG_object_tracker");
	_device_layers.push_back("VK_LAYER_LUNARG_param_tracker");

	_instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	//_instance_extension.push_back("VK_EXT_debug_report");
}

PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT = nullptr;
PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT = nullptr;

void Renderer::InitDebug()
{
	fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");
	fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT");
	if (fvkCreateDebugReportCallbackEXT == nullptr || fvkDestroyDebugReportCallbackEXT == nullptr)
	{
		assert(0 && "Vulkan ERROR: FAILED to fetch Debug function pointers.");
		std::exit(-1);
	}


	auto err = fvkCreateDebugReportCallbackEXT(_instance, &debug_callback_create_info, nullptr, &_debug_report);
	if (err != VK_SUCCESS)
	{
		assert(0 && "Vulkan ERROR: FAILED to create Debug report.");
		std::exit(-1);
	}
}


void Renderer::DeInitDebug()
{
	fvkDestroyDebugReportCallbackEXT(_instance, _debug_report, nullptr);
	_debug_report = VK_NULL_HANDLE;
}

