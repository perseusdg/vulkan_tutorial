// vulkan_tutorial.cpp : Defines the entry point for the application.
//

#include "vulkan_tutorial.h"

VkApplication::VkApplication(int32_t height, int32_t width, std::string vkapplicatonname) {
	this->vkheight = height;
	this->vkwidth = width;
	this->appname = vkapplicatonname;
}

void VkApplication::run() {
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();

}

void VkApplication::initVulkan() {
	
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
}

int32_t VkApplication::rateDeviceSuitability(VkPhysicalDevice device) {
	int32_t score = 0;
	VkPhysicalDeviceProperties vkDeviceProperties;
	VkPhysicalDeviceFeatures vkDeviceFeatures;
	vkGetPhysicalDeviceProperties(device, &vkDeviceProperties);
	vkGetPhysicalDeviceFeatures(device, &vkDeviceFeatures);
	if (vkDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 1000;
	}
	else {
		score += 50;
	}

	score += vkDeviceProperties.limits.maxImageDimension2D;

	if (!vkDeviceFeatures.geometryShader) {
		return 0;
	}

	return score;

}

QueueFamilyIndices VkApplication::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamily(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamily.data());
	
	uint32_t i = 0;
	for (const auto& queue : queueFamily) {
		if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vkSurface, &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
		}
		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

SwapChainSupportDetails VkApplication::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails swapChainDetails;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vkSurface, &swapChainDetails.capabilites);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &formatCount, nullptr);
	if (formatCount != 0) {
		swapChainDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &formatCount, swapChainDetails.formats.data());
	}
	uint32_t presentCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &presentCount, nullptr);
	if (presentCount != 0) {
		swapChainDetails.presentModes.resize(presentCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &presentCount, swapChainDetails.presentModes.data());
	}

	return swapChainDetails;
}

VkSurfaceFormatKHR VkApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VkApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VkApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilites)
{
	//swapextent is the resolution of the images to be drawn,it is almost always equal
	//to the resolution of the screen.The range of possible resolutions is defined in 
	//VkSurfaceCapabilitiesKHR structre
	if (capabilites.currentExtent.height != std::numeric_limits<uint32_t>::max()) {
		std::cerr << "[Vulkan Log] : input extent is already valid and defined";
		return capabilites.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		VkExtent2D actualExtent = { static_cast<uint32_t>(width),static_cast<uint32_t>(height) };
		actualExtent.height = std::clamp(actualExtent.height, capabilites.minImageExtent.height, capabilites.maxImageExtent.height);
		actualExtent.width = std::clamp(actualExtent.width, capabilites.minImageExtent.width, capabilites.maxImageExtent.width);

		return actualExtent;
	}
}

void VkApplication::pickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> vkPhysicalDevicesV(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, vkPhysicalDevicesV.data());
	std::multimap<int32_t, VkPhysicalDevice> candidates;
	for (const auto& device : vkPhysicalDevicesV) {
		int32_t score = rateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}
	if (candidates.rbegin()->first > 0) {
		if (isDeviceSuitable(candidates.rbegin()->second)) {
			vkPhysicalDevice = candidates.rbegin()->second;
		}
		else {
			throw std::runtime_error("couldn't find a suitable vulkan device 1");
		}
		

	}
	else {
		throw std::runtime_error("couldn't find a suitable vulkan device 2");
	}

}

bool VkApplication::isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);
	bool extensionsSupported = checkDeviceExtensionsSupport(device);
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainDetails = querySwapChainSupport(device);
		swapChainAdequate = !swapChainDetails.formats.empty() && !swapChainDetails.presentModes.empty();
	}
	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

void VkApplication::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(vkPhysicalDevice);
	std::set<uint32_t> uniqueGraphicsFamily = { indices.graphicsFamily.value(),indices.presentFamily.value() };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfosV;
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueGraphicsFamily) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfosV.push_back(queueCreateInfo);
	}
	VkPhysicalDeviceFeatures vkDeviceFeatures{};
	VkDeviceCreateInfo vkDeviceCreateInfo{};
	vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	vkDeviceCreateInfo.pQueueCreateInfos = queueCreateInfosV.data();
	vkDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfosV.size());
	vkDeviceCreateInfo.pEnabledFeatures = &vkDeviceFeatures;
	vkDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	vkDeviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	//lines 132-133 enabled VK_KHR_Swapchain support
	if (enableValidationLayers) {
		vkDeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		vkDeviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		vkDeviceCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(vkPhysicalDevice, &vkDeviceCreateInfo, nullptr, &vkDevice) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vulkan logical device");
	}

	vkGetDeviceQueue(vkDevice, indices.graphicsFamily.value(),0, &vkGraphicsQueue);
	vkGetDeviceQueue(vkDevice,indices.presentFamily.value(),0,&vkPresentQueue);
}

bool VkApplication::checkDeviceExtensionsSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableProperties(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableProperties.data());
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
	for (const auto& extension : availableProperties) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void VkApplication::createSwapChain()
{
	SwapChainSupportDetails supportDetails = querySwapChainSupport(vkPhysicalDevice);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(supportDetails.formats);
	VkPresentModeKHR presentModes = chooseSwapPresentMode(supportDetails.presentModes);
	VkExtent2D extent = chooseSwapExtent(supportDetails.capabilites);
	uint32_t minImageCount = supportDetails.capabilites.minImageCount;
	uint32_t imageCount = minImageCount + 1;

	if (supportDetails.capabilites.maxImageCount > 0 && imageCount > supportDetails.capabilites.maxImageCount) {
		imageCount = supportDetails.capabilites.maxImageCount;
	}

	VkSwapchainCreateInfoKHR vkCreateInfo{};
	vkCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	vkCreateInfo.surface = vkSurface;
	vkCreateInfo.minImageCount = imageCount;
	vkCreateInfo.imageFormat = surfaceFormat.format;
	vkCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	vkCreateInfo.imageExtent = extent;
	vkCreateInfo.imageArrayLayers = 1;
	//imageArrayLayers specifies the amount of layers each image consists of except 
	//incase of a 3d stereoscopic image..need to look into it
	vkCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	//imageUsage is used to directly render the image to swapchain,in order to 
	//render to an offscreen image use VK_IMAGE_USAGE_TRANSFER_DST_BIT

	QueueFamilyIndices indices = findQueueFamilies(vkPhysicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(),indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		vkCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		vkCreateInfo.queueFamilyIndexCount = 2;
		vkCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		vkCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vkCreateInfo.queueFamilyIndexCount = 0;
		vkCreateInfo.pQueueFamilyIndices = 0;
	}

	vkCreateInfo.preTransform = supportDetails.capabilites.currentTransform;
	vkCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	//the composite alpha specifies the alpha channel needed to blend with other windows 
	//in the window system
	vkCreateInfo.presentMode = presentModes;
	vkCreateInfo.clipped = VK_TRUE;
	//clipped mode correlates to if we care about the color of pixels that are not in the view
	vkCreateInfo.oldSwapchain = nullptr;

	if (vkCreateSwapchainKHR(vkDevice, &vkCreateInfo, nullptr, &vkSwapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain");
	}

	vkGetSwapchainImagesKHR(vkDevice, vkSwapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(vkDevice, vkSwapChain, &imageCount, swapChainImages.data());
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void VkApplication::createImageViews() {
	swapChainImageViews.resize(swapChainImages.size());

}

void VkApplication::createInstance() {
	//this is the first thing to do with a vulkan
	//library, create an instance that serves as a connection
	//between the application and vulkan library

	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested but not found");
	}
	VkApplicationInfo vkAppInfo{};
	vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vkAppInfo.pApplicationName = "vk app";
	vkAppInfo.applicationVersion = VK_API_VERSION_1_2;
	vkAppInfo.pEngineName = "no engine";
	vkAppInfo.engineVersion = VK_API_VERSION_1_2;
	vkAppInfo.apiVersion = VK_API_VERSION_1_2;

	//this next struct is mandatory, tells the program
	//which global extensions and validation layers can be used
	std::vector<const char*> extensions = getRequiredExtensions();
	VkInstanceCreateInfo vkCreateInfo{};
	vkCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkCreateInfo.pApplicationInfo = &vkAppInfo;
	vkCreateInfo.enabledExtensionCount = static_cast<int32_t>(extensions.size());
	vkCreateInfo.ppEnabledExtensionNames = extensions.data();
	VkDebugUtilsMessengerCreateInfoEXT vkDebugCreateInfo{};
	if (enableValidationLayers) {
		vkCreateInfo.enabledLayerCount = static_cast<int32_t>(validationLayers.size());
		vkCreateInfo.ppEnabledLayerNames = validationLayers.data();
		populateDebugMessengerCreateInfo(vkDebugCreateInfo);
		vkCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &vkDebugCreateInfo;
		
	}
	else {
		vkCreateInfo.enabledLayerCount = 0;
		vkCreateInfo.pNext = nullptr;
	}
	vkCreateInfo.enabledLayerCount = 0;
	if (vkCreateInstance(&vkCreateInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vulkan instance");
	}
	

}

bool VkApplication::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layername : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layername, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}

	}

	return true;
}


void VkApplication::initWindow() {
	//glfwinit is called first to initialize 
	//glfw, the glfwcreatewindow is called with
	//GLFW_CLIENT_API and GLFW_NO_API ..to support
	//vulkan
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);//this prevenets the window form resizing
	window = glfwCreateWindow(vkwidth, vkheight, appname.c_str(), nullptr, nullptr);
}

void VkApplication::mainLoop() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

}

void VkApplication::cleanup() {
	vkDestroySwapchainKHR(vkDevice, vkSwapChain, nullptr);
	vkDestroyDevice(vkDevice, nullptr);
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, vkDebugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(instance, vkSurface, nullptr);
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}

void VkApplication::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &vkSurface) != VK_SUCCESS) {
		throw std::runtime_error("unable to create window surface");
	}
}

void VkApplication::setupDebugMessenger() {
	if (!enableValidationLayers) return;
	VkDebugUtilsMessengerCreateInfoEXT vkDebugCreateInfo;
	populateDebugMessengerCreateInfo(vkDebugCreateInfo);
	//the VkDebugUtilsMessengerCreateInfoExt needs to be passed to the function
	//vkCreateDebugUtilsMessengerEXT , but since it is not loaded by default
	//it needs to be loaded using vkGetInstanceProcAddr
	if (CreateDebugUtilsMessengerEXT(instance, &vkDebugCreateInfo, nullptr, &vkDebugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to create debug messenger ext");
	}
}

VkResult VkApplication::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* debugMsgInfo, VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, debugMsgInfo, pAllocator, pDebugMessenger);

	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

std::vector<const char*> VkApplication::getRequiredExtensions(){
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VkApplication::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "validation Layer : " << pCallbackData->pMessage << std::endl;
	//callback should always be false,if callback returns true then
	//it is aborted with VK_ERROR_VALIDATION_FAILED_EXT
	return VK_FALSE;
}

void VkApplication::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT pDebugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerExt");
	if (func != nullptr) {
		func(instance, pDebugMessenger, pAllocator);
	}
	
}

void VkApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& vkDebugCreateInfo)
{
	vkDebugCreateInfo = {};
	vkDebugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
	vkDebugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	vkDebugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	vkDebugCreateInfo.pfnUserCallback = debugCallback;
	vkDebugCreateInfo.pUserData = nullptr;
}


int main(){
	VkApplication vkApp(600, 800, "vkapp");
	try {
		vkApp.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return(EXIT_FAILURE);
	}

	return(EXIT_SUCCESS);
}