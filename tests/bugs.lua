#!/usr/bin/env tarantool

-- Those lines of code are for debug purposes only
-- So you have to ignore them
-- {{
package.preload['curl.driver'] = '../curl/driver.so'
-- }}

local os = require('os')

box.cfg {}

function run(is_skipping, desc, func)
  print('Running', desc)
  if not is_skipping then
    if not func() then
      os.exit(1)
    end
  else
    print('SKIP')
  end
end

run(false, 'Issus https://github.com/tarantool/curl/issues/3', function()
  local curl = require('curl')
  local json = require('json')
  local http = curl.http()
  local data = {a = 'b'}
  local headers = {}
  headers['Content-Type'] = 'application/json'
  local res = http:post('https://httpbin.org/post', json.encode(data),
                        {headers=headers})
  assert(res.code == 200)
  assert(json.decode(res.body)['json']['a'] == data['a'])
  http:free()
  return true
end)

print('[+] bugs OK')

os.exit(0)
