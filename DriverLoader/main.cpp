#include "pe.hpp"
#include "nt.hpp"
#include "interface.hpp"
#include "intel.hpp"
#include "service.hpp"
#include <fstream>

std::vector<uint8_t> getFile(LPCSTR fileName)
{
	std::ifstream file(fileName, std::ios::in | std::ios::binary | std::ios::ate);
	std::vector<uint8_t> contents;
	if (file.is_open())
	{
		file.unsetf(std::ios::skipws);
		file.seekg(0, std::ios::beg);
		contents.reserve(file.tellg());
		contents.insert(contents.begin(), std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());
		file.close();
	}

	return contents;
}

int main(int arg_count, char* args[])
{
	if (arg_count < 2)
	{
		std::cout << "Please supply a file to load" << std::endl;
		return 0;
	}

	DriverLoader loader(L"Intel3");
	if (!loader.load_driver((char*)intel::intel_driver, sizeof(intel::intel_driver)))
		std::cout << "failed to load driver" << std::endl;
	else
	{
		KeInterface keInterface;
		std::vector<byte> file = getFile(args[1]);
		PEImage image(file);
		PVOID pImageBase;
		if (!keInterface.callFunction("ntoskrnl.exe", "ExAllocatePool", &pImageBase, nt::NonPagedPool, image.getSize()))
			std::cout << "failed to allocate pool" << std::endl;

		uintptr_t image_base = uintptr_t(pImageBase);
		const auto _get_import = [&keInterface](const char* lib_name, const char* module_name)
		{
			uintptr_t module_base = keInterface.getKernelModuleBase(lib_name);
			return keInterface.getModuleExport(module_name, module_base);
		};

		image.processRelocations(image_base);
		image.resolveImports(_get_import);
		image.mapImage();
		std::cout << "Mapped Image: " << std::hex << image_base << std::endl;

		keInterface.writeMemory(uintptr_t(image.getMappedImage()), uintptr_t(pImageBase), image.getSize());
		uintptr_t entry_address = image_base + image.getEntryPoint();

		NTSTATUS nt_status;
		if (!keInterface.callFunction(entry_address, &nt_status, 0, 0) || !NT_SUCCESS(nt_status))
			std::cout << "failed to call entry" << std::endl;
			
		if (!loader.unload_driver())
			std::cout << "failed to unload driver" << std::endl;
			
		loader.clean_up();
	}

	while (!GetAsyncKeyState(VK_END));
	return 0;
}