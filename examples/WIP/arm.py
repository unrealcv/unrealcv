# A virtual robot arm demo to show blueprint interop, domain-adaptation and collision event
from unrealcv import client

def main():
    client.connect()
    client.request('vexec ArmController_C_0 ToggleRandomPose')

if __name__ == '__main__':
    main()
