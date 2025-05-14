# This script will only runs if the container was created in root mode

# In main.c, it is passed the containers real pid in root_namespace 
pid_container=$1

# Is pid parameter alright?
if [ -z "$pid_container" ]; then
    echo "Error: Container's PID not found!"
    exit 1
fi

# It is not recommended to use the real pid like a identifier, but for the purpose of this project we will ignore
# Important: interfaces can only have 15 bytes lenght of name
ip link add ${pid_container}_br type veth peer name ${pid_container}_ns

# Estivador know will connect with the new container by this veth
ovs-vsctl add-port Estivador ${pid_container}_br

# Container know have a net interface connected with the bridge
ip link set ${pid_container}_ns netns $pid_container

# Activating the veths
# Warning, Why this line is commented? This will do an error, because the veth is right know inside the container (inside the network namespace),
# so we have to up the veth inside the container, not in the root namespace
# ip link set ${pid_container}_ns up
ip link set ${pid_container}_br up
