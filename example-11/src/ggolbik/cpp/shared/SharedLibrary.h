#pragma once

#include <dlfcn.h>  // ::dlopen

#include <filesystem>  // std::filesystem::exists
#include <iostream>    // std::cout; std::endl
#include <memory>      // std::shared_ptr
#include <string>      // std::string

namespace ggolbik {
namespace cpp {
namespace shared {

/**
 * @brief Shared library must not be destroyed before all created instances are
 * destroyed.
 *
 * https://tldp.org/HOWTO/html_single/C++-dlopen/
 *
 * There are a few things to note when loading classes:
 *
 * - You must provide both a creation and a destruction function; you must not
 * destroy the instances using delete from inside the executable, but always
 * pass it back to the module. This is due to the fact that in C++ the operators
 * new and delete may be overloaded; this would cause a non-matching new and
 * delete to be called, which could cause anything from nothing to memory leaks
 * and segmentation faults. The same is true if different standard libraries are
 * used to link the module and the executable.
 *
 * - The destructor of the interface class should be virtual in any case. There
 * might be very rare cases where that would not be necessary, but it is not
 * worth the risk, because the additional overhead can generally be ignored. If
 * your base class needs no destructor, define an empty (and virtual) one
 * anyway; otherwise you will have problems sooner or later; I can guarantee you
 * that.
 *
 * @tparam T The name of the instance that will be created from the library.
 */
template <class T>
class SharedLibrary {
 public:
  /**
   * @brief Definition of the create method.
   */
  using CreateMethod = T* (*)();
  /**
   * @brief Definition of the destroy method.
   */
  using DestroyMethod = void (*)(T*);
  /**
   * @brief Definition of the instance pointer.
   */
  using InstancePtr = std::shared_ptr<T>;

 public:
  /**
   * @brief Construct a new Shared Library object.
   *
   * @param libraryFilename The filename of the library.
   */
  explicit SharedLibrary(const std::string& libraryFilename)
      : SharedLibrary(libraryFilename, "create", "destroy") {
    // empty
  }
  /**
   * @brief Construct a new Shared Library object.
   *
   * @param libraryFilename The filename of the library.
   * @param createFunctionName The name of the create method.
   * @param destroyFunctionName The name of the destroy method.
   */
  explicit SharedLibrary(const std::string& libraryFilename,
                         const std::string& createFunctionName,
                         const std::string& destroyFunctionName)
      : libraryFilename(libraryFilename),
        createFunctionName(createFunctionName),
        destroyFunctionName(destroyFunctionName) {
    // empty
  }
  SharedLibrary(const SharedLibrary& arg) = delete;
  SharedLibrary& operator=(const SharedLibrary& arg) = delete;
  /**
   * @brief Destroy the Shared Library object
   */
  ~SharedLibrary() { this->unload(); };

 public:
  /**
   * @brief Loads the library.
   *
   * @return Whether the library could be loaded.
   */
  bool load() {
    std::string errorMessage;
    return this->load(errorMessage);
  }
  /**
   * @brief Loads the library.
   *
   * @param errorMessage Contains a message in case of an error.
   * @return Whether the library could be loaded.
   */
  bool load(std::string& errorMessage) {
    errorMessage = "";
    if (this->libraryHandle != nullptr) {
      // library is already loaded.
      return true;
    }
    // reset errors
    ::dlerror();

    // check if library exists
    std::filesystem::path libraryPath(this->libraryFilename);
    if (!std::filesystem::exists(libraryPath)) {
      errorMessage = "Library could not be found.";
      return false;
    }

    // load library
    std::cout << "Loading library: " << this->libraryFilename << std::endl;
    this->libraryHandle = ::dlopen(libraryFilename.c_str(), RTLD_LAZY);
    if (this->libraryHandle == nullptr) {
      errorMessage = "Library could not be opened.";
      char* error = ::dlerror();
      if (error != nullptr) {
        errorMessage += " " + std::string(error);
      }
      return false;
    }
    std::cout << "Loaded library: " << this->libraryFilename << std::endl;

    return true;
  }
  /**
   * @brief Unloads the library. Should not be executed before all created
   * instances are destroyed.
   *
   * @return Whether the unload operation was successful.
   */
  bool unload() {
    std::string errorMessage;
    return this->unload(errorMessage);
  }
  /**
   * @brief Unloads the library. Should not be executed before all created
   * instances are destroyed.
   *
   * @param errorMessage Contains a message in case of an error.
   * @return Whether the unload operation was successful.
   */
  bool unload(std::string& errorMessage) {
    errorMessage = "";
    if (this->libraryHandle != nullptr) {
      // reset errors
      ::dlerror();

      // close library
      std::cout << "Closing library: " << this->libraryFilename << std::endl;
      int result = ::dlclose(this->libraryHandle);
      if (result != 0) {
        errorMessage = "Library could not be closed.";
        char* error = ::dlerror();
        if (error != nullptr) {
          errorMessage += " " + std::string(error);
        }
        // undefined behavior. Remove reference to methods.
        this->createHandle = nullptr;
        this->destroyHandle = nullptr;
        return false;
      } else {
        // clear references
        this->createHandle = nullptr;
        this->destroyHandle = nullptr;
        this->libraryHandle = nullptr;
        std::cout << "Closed library: " << this->libraryFilename << std::endl;
        return true;
      }
    }
    return true;
  }
  /**
   * @brief Creates a new instance.
   *
   * @return The created instance or null in case of an error.
   */
  InstancePtr create() {
    std::string errorMessage;
    return this->create(errorMessage);
  }
  /**
   * @brief Creates a new instance.
   *
   * @param errorMessage Contains a message in case of an error.
   * @return The created instance or null in case of an error.
   */
  InstancePtr create(std::string& errorMessage) {
    errorMessage = "";
    InstancePtr instance;
    if (this->libraryHandle == nullptr) {
      errorMessage = "Library is not loaded.";
      return instance;
    }

    // reset errors
    ::dlerror();

    if (this->createHandle == nullptr) {
      // load the symbol of create method
      std::cout << "Loading create method: " << this->libraryFilename
                << std::endl;
      this->createHandle = reinterpret_cast<CreateMethod>(
          ::dlsym(this->libraryHandle, createFunctionName.c_str()));
      if (this->createHandle == nullptr) {
        errorMessage = "Create method could not be found.";
        char* error = ::dlerror();
        if (error != nullptr) {
          errorMessage += " " + std::string(error);
        }
        return instance;
      }
      std::cout << "Loaded create method: " << this->libraryFilename
                << std::endl;
    }
    if (this->destroyHandle == nullptr) {
      // load the symbol of destroy method
      std::cout << "Loading destroy method: " << this->libraryFilename
                << std::endl;
      this->destroyHandle = reinterpret_cast<DestroyMethod>(
          ::dlsym(this->libraryHandle, destroyFunctionName.c_str()));
      if (this->destroyHandle == nullptr) {
        errorMessage = "Destroy method could not be found.";
        char* error = ::dlerror();
        if (error != nullptr) {
          errorMessage += " " + std::string(error);
        }
        return instance;
      }
      std::cout << "Loaded destroy method: " << this->libraryFilename
                << std::endl;
    }

    // create instance with custom deleter
    std::cout << "Creating instance: " << this->libraryFilename << std::endl;
    instance = InstancePtr(this->createHandle(), [&](T* p) {
      std::cout << "Destroying instance: " << this->libraryFilename
                << std::endl;
      if (p != nullptr && this->destroyHandle != nullptr) {
        this->destroyHandle(p);
        std::cout << "Destroyed instance: " << this->libraryFilename
                  << std::endl;
      }
    });
    if (!instance) {
      errorMessage = "Failed to create instance.";
      return instance;
    }
    std::cout << "Created instance: " << this->libraryFilename << std::endl;

    return instance;
  }

 private:
  /**
   * @brief The filename of the library.
   */
  std::string libraryFilename;
  /**
   * @brief The name of the create method to be used for creation of a new
   * instance.
   */
  std::string createFunctionName;
  /**
   * @brief The name of the destroy method to be used for destruction of an
   * instance.
   */
  std::string destroyFunctionName;
  /**
   * @brief Reference to the loaded library.
   */
  void* libraryHandle = nullptr;
  /**
   * @brief Reference to the found create method.
   */
  CreateMethod createHandle = nullptr;
  /**
   * @brief Reference to the found destroy method.
   */
  DestroyMethod destroyHandle = nullptr;
};

}  // namespace shared
}  // namespace cpp
}  // namespace ggolbik