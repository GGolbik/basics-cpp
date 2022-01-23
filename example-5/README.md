# Basics C++ - Example 5

# Table of Contents

* [Build Project](#build-project)
* [SonarQube](#sonarqube)
* [Install Project](#install-project)
* [Build and Install with Docker](#build-and-install-with-docker)

# Build Project

Execute the `build.bat` script on Windows or `build.sh` script on Linux.

# SonarQube

Execute the `sonarqube.sh` script on Linux. There is no support for Windows yet.

The script will build the docker container with the CXX plugin, creates an instance of the container and will run the code analysis.
On first run the code analysis will fail. 
You have to configure the SonarQube server and change the token in the `sonarqube.sh` script.

Further configurations can be done in the `sonar-project.properties` file (see the [docs](
https://docs.sonarqube.org/latest/analysis/analysis-parameters/)).

# Install Project

Execute the `install.sh` script on Linux. There is no support for Windows yet.

# Build and Install with Docker

Execute the `docker.sh` script on Linux. There is no support for Windows yet.
