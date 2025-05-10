# This script create the interface that connect our containers with internet
# Make sure that you are running this file in root mode
# Thanks to Vinicius who helped me with this tutorial: https://blog.cloudlabs.ufscar.br/virt/network-virtualization/#servidor-dhcp

# Create bridge called "Estivador" and give it an IP
ovs-vsctl add-br Estivador
ip addr add 192.168.100.1/24 dev Estivador
ip link set Estivador up

# Enable ip forwarding
sysctl -w net.ipv4.ip_forward=1

# Enabling NAT with the 192.168.100.0/24 net 
sudo iptables -t nat -A POSTROUTING -s 192.168.100.0/24 -o wlp2s0 -j MASQUERADE
sudo iptables -A FORWARD -i Estivador -o wlp2s0 -j ACCEPT
sudo iptables -A FORWARD -i wlp2s0 -o Estivador -m state --state RELATED,ESTABLISHED -j ACCEPT

# Enabling DHCP for our containers
# Every container that is created will receive an ip thanks to this protocol
cat > /etc/dnsmasq.d/estivador.conf <<EOF
interface=Estivador
bind-interfaces
dhcp-range=192.168.100.100,192.168.100.200,12h
EOF

systemctl restart dnsmasq
systemctl enable dnsmasq

