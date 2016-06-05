addpath('../matlab');
requests = {'hi', 'hello', 'asdf'};

for index = 1:length(requests)
  req = requests{index};
  response = request(req);
  msg = sprintf('expect: %s, got %s', req, response);
  assert(isequal(response, req), msg);
end
