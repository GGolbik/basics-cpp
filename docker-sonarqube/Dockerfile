# Use SonarQube 8 community edition which is supported by the CXX plugin
FROM sonarqube:8-community

# Defines name of C++/CXX plugin
ARG PLUGIN_SONAR_CXX_FILENAME=sonar-cxx-plugin.jar

COPY ${PLUGIN_SONAR_CXX_FILENAME} /opt/sonarqube/extensions/plugins

# Define arguments used in this dockerfile
ARG LABEL_CREATED

# Set labels - OCI Image Format https://www.opencontainers.org/
LABEL \
  # version of the packaged software
  org.opencontainers.image.version="1.0.0" \
  # date and time on which the image was built (string, date-time as defined by RFC 3339).
  org.opencontainers.image.created=${LABEL_CREATED} \
  # Name of the distributing entity, organization or individual.
  org.opencontainers.image.vendor="GGolbik" \
  # Text description of the image.
  org.opencontainers.image.description="SonarQube"

