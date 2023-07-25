# Use the Nginx image from Docker Hub
FROM nginx:stable-alpine

# Remove the default Nginx configuration file
RUN rm /etc/nginx/conf.d/default.conf

# Add a new configuration file
ARG conf_file
COPY $conf_file /etc/nginx/conf.d/nginx.conf

COPY conf-files/index.html /usr/share/nginx/html/index.html