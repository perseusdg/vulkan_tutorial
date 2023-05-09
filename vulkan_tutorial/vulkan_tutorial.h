// vulkan_tutorial.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#define GLFW_INCLUDE_VULKAN

#include <iostream>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <map>
#include <optional>
#include <vector>
#include <set>
#include <algorithm>
#include <limits>
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilites;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


class VkApplication {
public:
	VkApplication(int32_t height, int32_t width, std::string vkapplicatonname);
	void run();

private:
	void initVulkan();
	bool checkValidationLayerSupport();
	void initWindow();
	void mainLoop();
	void createInstance();
	void cleanup();
	void createSurface();
	void setupDebugMessenger();
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	void createLogicalDevice();
	bool checkDeviceExtensionsSupport(VkPhysicalDevice device);
	void createSwapChain();
	void createImageViews();

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* debugMsgInfo,
		VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);

	std::vector<const char*> getRequiredExtensions();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	);

	void DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT pDebugMessenger,
		const VkAllocationCallbacks* pAllocator);

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& vkDebugCreateInfo);

	int32_t rateDeviceSuitability(VkPhysicalDevice device);

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilites);



	GLFWwindow* window;
	int32_t vkheight;
	int32_t vkwidth;
	std::string appname;
	VkInstance instance;
	VkDebugUtilsMessengerEXT vkDebugMessenger;
	VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
	VkDevice vkDevice;
	VkQueue vkGraphicsQueue;
	VkSurfaceKHR vkSurface;
	VkQueue vkPresentQueue;
	VkSwapchainKHR vkSwapChain;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;


};
// TODO: Reference additional headers your program requires here.
//swapchain -  vulkan doesnt have a default frame buffer so it requires and 
//infrastructure