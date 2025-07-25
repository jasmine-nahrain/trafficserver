'''
'''
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

Test.Summary = '''
Test interaction of H2 and chunked encoding
'''
Test.SkipIf(Condition.CurlUsingUnixDomainSocket())

Test.SkipUnless(
    Condition.HasProgram("nghttp", "Nghttp need to be installed on system for this test to work"),
    Condition.HasCurlFeature('http2'))
Test.ContinueOnFail = True

Test.GetTcpPort("upstream_port")
Test.GetTcpPort("upstream_port2")
Test.GetTcpPort("upstream_port3")

# Define default ATS
ts = Test.MakeATSProcess("ts", enable_tls=True)

# add ssl materials like key, certificates for the server
ts.addDefaultSSLFiles()

delay_server = Test.Processes.Process(
    "delay-server", "bash -c '" + Test.TestDirectory + "/delay-server.sh {} outserver1'".format(Test.Variables.upstream_port))
server2 = Test.Processes.Process(
    "server2", "bash -c '" + Test.TestDirectory + "/server2.sh {} outserver2'".format(Test.Variables.upstream_port2))
server3 = Test.Processes.Process(
    "server3", "bash -c '" + Test.TestDirectory + "/server3.sh {} outserver3'".format(Test.Variables.upstream_port3))

ts.Disk.records_config.update(
    {
        'proxy.config.diags.debug.enabled': 1,
        'proxy.config.diags.debug.tags': 'http',
        'proxy.config.ssl.server.cert.path': '{0}'.format(ts.Variables.SSLDir),
        'proxy.config.ssl.server.private_key.path': '{0}'.format(ts.Variables.SSLDir),
        'proxy.config.ssl.client.verify.server.policy': 'PERMISSIVE',
    })

ts.Disk.remap_config.AddLine('map /delay-chunked-response http://127.0.0.1:{0}'.format(Test.Variables.upstream_port))
ts.Disk.remap_config.AddLine('map /post-full http://127.0.0.1:{0}'.format(Test.Variables.upstream_port2))
ts.Disk.remap_config.AddLine('map /post-chunked http://127.0.0.1:{0}'.format(Test.Variables.upstream_port3))

ts.Disk.ssl_multicert_config.AddLine('dest_ip=* ssl_cert_name=server.pem ssl_key_name=server.key')

# Using netcat as a cheap origin server in case 1 so we can insert a delay in sending back the response.
# Replaced microserver for cases 2 and 3 as well because I was getting python exceptions when running
# microserver if chunked encoding headers were specified for the request headers

# H2 GET request
# chunked response without content-length
# delay before final chunk size
server1_out = Test.Disk.File("outserver1")
tr = Test.AddTestRun()
tr.Processes.Default.Command = 'nghttp -vv https://127.0.0.1:{}/delay-chunked-response'.format(ts.Variables.ssl_port)
tr.Processes.Default.ReturnCode = 0
tr.Processes.Default.StartBefore(Test.Processes.ts)
tr.Processes.Default.StartBefore(delay_server)
tr.Processes.Default.Streams.All = Testers.ExcludesExpression("RST_STREAM", "Delayed chunk close should not cause reset")
tr.Processes.Default.Streams.All += Testers.ExcludesExpression("< content-length", "Should return chunked")
tr.Processes.Default.Streams.All += Testers.ContainsExpression(":status: 200", "Should get successful response")
tr.StillRunningAfter = ts
# No resets in the output
# No content lengths in the header

# HTTP2 POST: www.example.com Host, chunked body
server2_out = Test.Disk.File("outserver2")
tr = Test.AddTestRun()
tr.Processes.Default.StartBefore(server2)
tr.MakeCurlCommand(
    '--http2 -k https://127.0.0.1:{}/post-full --verbose -H "Transfer-encoding: chunked" -d "Knock knock"'.format(
        ts.Variables.ssl_port),
    ts=ts)
tr.Processes.Default.ReturnCode = 0
tr.Processes.Default.Streams.All = Testers.ContainsExpression("HTTP/2 200", "Request should succeed")
tr.Processes.Default.Streams.All += Testers.ContainsExpression("< content-length:", "Response should include content length")
server2_out = Testers.ContainsExpression("Transfer-Encoding: chunked", "Request should be chunked encoded")
# No content-length in header
# Transfer encoding to origin, but no content-length
# Content length on the way back

# HTTP2 POST: chunked post body and chunked response
server3_out = Test.Disk.File("outserver3")
tr = Test.AddTestRun()
tr.Processes.Default.StartBefore(server3)
tr.MakeCurlCommand(
    '--http2 -k https://127.0.0.1:{}/post-chunked --verbose -H "Transfer-encoding: chunked" -d "Knock knock"'.format(
        ts.Variables.ssl_port),
    ts=ts)
tr.Processes.Default.ReturnCode = 0
tr.Processes.Default.Streams.All = Testers.ContainsExpression("HTTP/2 200", "Request should succeed")
tr.Processes.Default.Streams.All += Testers.ExcludesExpression("< content-length:", "Response should not include content length")
server3_out = Testers.ContainsExpression("Transfer-Encoding: chunked", "Request should be chunked encoded")
# No content length in header
# Transfer encoding to origin, but no content-length
# No content length in the response
