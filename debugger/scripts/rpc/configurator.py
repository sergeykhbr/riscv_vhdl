"""
  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
"""
RPC_CONFIGURATOR_DEBUG = 0

class RpcValueType(object):
    def __init__(self, client, objname):
        self.client = client
        self.objname = objname
        self._value = None

    def value2str(self, val):
        return str(val)

    @property
    def value(self):
        return self._value

    @value.setter
    def value(self, val):
        req = ["Command", "{0} {1}".format(self.objname, self.value2str(val))]
        self.client.send(req)

    @value.getter
    def value(self):
        req = ["Command", "{0}".format(self.objname)]
        return self.client.send(req)


class RpcFloatType(RpcValueType):
    def __init__(self, client, objname):
        RpcValueType.__init__(self, client, objname)

    def value2str(self, val):
        return str(float(val))


class RpcKeypadType(RpcValueType):
    def __init__(self, client, objname):
        RpcValueType.__init__(self, client, objname)

    def __repr__(self):
        return self.value

    def value2str(self, val):
        t1 = {False: 'release', True: 'press'}
        return t1[bool(val)]

    def press(self):
        self.value = True

    def release(self):
        self.value = False


class RpcDisplayType(object):
    def __init__(self, client, objname):
        self.client = client
        self.objname = objname
        self._frame = None
        self._config = {"Width": 0, "Height": 0, "BkgColor": 0x0}

    @property
    def frame(self):
        return self._frame

    @frame.setter
    def frame(self, value):
        pass

    @frame.getter
    def frame(self):
        req = ["Command", "{0} frame encoded".format(self.objname)]
        return self.client.send(req)

    @property
    def config(self):
        return self._config

    @config.setter
    def config(self, value):
        pass

    @config.getter
    def config(self):
        req = ["Command", "{0} config".format(self.objname)]
        return self.client.send(req)

    def width(self):
        return self.config["Width"]

    def height(self):
        return self.config["Height"]

    def BkgColor(self):
        return self.config["BkgColor"]


class PlatformConfig(object):
    def __init__(self, client):
        self.client = client

    def instantiate(self, rpc):
        req = ["Configuration", "platform"]
        resp = self.client.send(req)
        if "Name" not in resp:
            raise ValueError('Wrong platform configuration: {0}'.format(resp))

        rpc.__dict__["Name"] = resp['Name']
        if RPC_CONFIGURATOR_DEBUG:
            print("Detected Device: %s" % resp['Name'])

        # Check display in configuration:
        if "Display" in resp:
            rpc.__dict__[resp["Display"]] = \
                RpcDisplayType(self.client, resp["Display"])
            if RPC_CONFIGURATOR_DEBUG:
                print("Display: %s" % resp['Display'])

        if "Vars" in resp:
            if RPC_CONFIGURATOR_DEBUG:
                print("Platform variables:")
            for v in resp["Vars"]:
                if RPC_CONFIGURATOR_DEBUG:
                    print("    {0} of type({1}): {2}".format(v[0], v[1], v[2]))
                if v[1] == 'float':
                    rpc.__dict__[v[0]] = RpcFloatType(self.client, v[0])
                else:
                    rpc.__dict__[v[0]] = RpcValueType(self.client, v[0])
                    print("    warning: unsupported Variable type %s" % v[1])

        if "Keys" in resp:
            if RPC_CONFIGURATOR_DEBUG:
                print("Keyboard:")
            for v in resp["Keys"]:
                if RPC_CONFIGURATOR_DEBUG:
                    print("    Key: {0}".format(v))
                rpc.__dict__[v] = RpcKeypadType(self.client, v)
