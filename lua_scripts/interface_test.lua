local nk = require("moonnuklear")

function test(ctx)
  print(ctx)
  print(ctx:font())
  print(ctx:font():height())
end
