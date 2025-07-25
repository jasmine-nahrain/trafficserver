#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

import os

Test.Summary = '''
Test TS API Hooks.
'''

Test.SkipUnless(Condition.HasCurlFeature('http2'),)
Test.ContinueOnFail = True

server = Test.MakeOriginServer("server")

request_header = {"headers": "GET /argh HTTP/1.1\r\nHost: doesnotmatter\r\n\r\n", "timestamp": "1469733493.993", "body": ""}
response_header = {"headers": "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n", "timestamp": "1469733493.993", "body": ""}
server.addResponse("sessionlog.json", request_header, response_header)

# Disable the cache to make sure each request is forwarded to the origin
# server.
ts = Test.MakeATSProcess("ts", enable_tls=True, enable_cache=False)

# test_hooks.so will output test logging to this file.
log_path = os.path.join(ts.Variables.LOGDIR, "log.txt")
Test.Env["OUTPUT_FILE"] = log_path

ts.addDefaultSSLFiles()

ts.Disk.records_config.update(
    {
        'proxy.config.proxy_name': 'Poxy_Proxy',  # This will be the server name.
        'proxy.config.ssl.server.cert.path': '{0}'.format(ts.Variables.SSLDir),
        'proxy.config.ssl.server.private_key.path': '{0}'.format(ts.Variables.SSLDir),
        'proxy.config.url_remap.remap_required': 0,
        'proxy.config.diags.debug.enabled': 0,
        'proxy.config.diags.debug.tags': 'http|test_hooks',
    })

ts.Disk.ssl_multicert_config.AddLine('dest_ip=* ssl_cert_name=server.pem ssl_key_name=server.key')

Test.PrepareTestPlugin(os.path.join(Test.Variables.AtsTestPluginsDir, 'test_hooks.so'), ts)

ts.Disk.remap_config.AddLine("map http://one http://127.0.0.1:{0}".format(server.Variables.Port))
ts.Disk.remap_config.AddLine("map https://one http://127.0.0.1:{0}".format(server.Variables.Port))

ipv4flag = ""
if not Condition.CurlUsingUnixDomainSocket():
    ipv4flag = "--ipv4"

tr = Test.AddTestRun()
# Probe server port to check if ready.
tr.Processes.Default.StartBefore(server, ready=When.PortOpen(server.Variables.Port))
tr.Processes.Default.StartBefore(Test.Processes.ts)
#
tr.MakeCurlCommand('--verbose {0} --header "Host: one" http://localhost:{1}/argh'.format(ipv4flag, ts.Variables.port), ts=ts)
tr.Processes.Default.ReturnCode = 0

if not Condition.CurlUsingUnixDomainSocket():
    tr = Test.AddTestRun()
    tr.MakeCurlCommand(
        '--verbose --ipv4 --http2 --insecure --header "Host: one" https://localhost:{0}/argh'.format(ts.Variables.ssl_port), ts=ts)
    tr.Processes.Default.ReturnCode = 0

    tr = Test.AddTestRun()
    tr.MakeCurlCommand(
        '--verbose --ipv4 --http1.1 --insecure --header "Host: one" https://localhost:{0}/argh'.format(ts.Variables.ssl_port),
        ts=ts)
    tr.Processes.Default.ReturnCode = 0

# The probing of the ATS port to detect when ATS is ready may be seen by ATS as a VCONN start/close, so filter out these
# events from the log file.
#
tr = Test.AddTestRun()
tr.Processes.Default.Command = "cd " + Test.RunDirectory + " ; . " + Test.TestDirectory + "/clean.sh"
tr.Processes.Default.ReturnCode = 0

tr = Test.AddTestRun()
tr.Processes.Default.Command = "echo check log"
tr.Processes.Default.ReturnCode = 0
f = tr.Disk.File(log_path)
if Condition.CurlUsingUnixDomainSocket():
    f.Content = "log_uds.gold"
else:
    f.Content = "log.gold"
    f.Content += Testers.ContainsExpression("Global: event=TS_EVENT_VCONN_CLOSE", "VCONN_CLOSE should trigger 2 times")
