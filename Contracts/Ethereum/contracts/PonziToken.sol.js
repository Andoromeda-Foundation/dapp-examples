import 'openzeppelin-solidity/contracts/token/ERC20/StandardToken.sol

contract TradeableToken is StandardToken{
  ...
}

contract ShareableToken is TradeableToken{
  ...
}

contract PonziToken is ShareableToken{
  ...
}
