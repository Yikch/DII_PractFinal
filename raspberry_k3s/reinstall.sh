/usr/local/bin/k3s-uninstall.sh
curl -sfL https://get.k3s.io | INSTALL_K3S_EXEC="--node-name raspberrypi.local" sh -
cd registry_podman 
podman kill mi-registry
podman rm mi-registry
podman compose up -d
cd ..
sudo kubectl apply -f deployment/mosquitto.yaml
sleep 15
sudo kubectl apply -f deployment/alertador-videollamada-telegram.yaml
