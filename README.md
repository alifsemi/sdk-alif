# Alif Semiconductor Zephyr SDK

Welcome to the **Alif Semiconductor Zephyr SDK**, a comprehensive environment for developing applications on Alif's devices using the Zephyr RTOS. This SDK provides Alif-specific code samples and the essential `west.yml` file, which manages all required modules and submodules for a seamless development experience.

The SDK is designed to help developers configure, build, and deploy real-time applications on Alif’s low-power, high-performance devices.

## User Guide

The **User Guide** offers detailed instructions on setting up, building, and deploying applications on Alif devices using the Zephyr RTOS.

Access the latest version of the guide at: [Alif Zephyr SDK User Guide](https://github.com/alifsemi/sdk-alif/releases/download/v1.2.0/user_guide.pdf).

## Release Notes

Stay updated with the latest changes:

- **New Features**: Explore the latest additions and capabilities in the current release.
- **Fixed Bugs**: Review resolved issues for improved performance and stability.
- **Known Issues**: Be aware of current limitations or potential challenges during development.

For complete details on the latest released version, see the [Release Notes](https://github.com/alifsemi/sdk-alif/releases/download/v1.2.0/release_notes.pdf).

## Getting the Alif SDK

To obtain or update the Alif SDK, you need **Git** and **West** installed.

### Clone/Check-out a New SDK Delivery

Execute below commands to achieve the whole SDK delivery at revision `${revision}` and place it in a folder named `sdk-alif`:

```bash
mkdir sdk-alif
cd sdk-alif
west init -m https://github.com/alifsemi/sdk-alif.git --mr ${revision}
west update
```
Replace `${revision}` with any SDK revision(branch/tag/commit SHA) you wish to achieve. This can be `main` if you want the latest state, or any commit SHA or tag.

**Syntax:**
```west init -m <URL> --mr <REVISION>```

**Example:**
```west init -m https://github.com/alifsemi/sdk-alif.git --mr v1.2.0```

### Getting Started

To explore the project and start building applications, see the [Building Application Binaries Guide](https://github.com/alifsemi/sdk-alif/blob/main/doc/user_guide/source/building_application_binaries.rst).
