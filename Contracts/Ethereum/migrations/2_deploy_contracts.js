var Counter = artifacts.require("./Counter.sol");
var PowhToken = artifacts.require("./PowhToken.sol");

var TradeableToken = artifacts.require("./PonziToken/TradeableToken.sol");
var ProfitableToken = artifacts.require("./PonziToken/ProfitableToken.sol");

module.exports = function(deployer) {
    deployer.deploy(Counter);
    deployer.deploy(PowhToken);
    deployer.deploy(TradeableToken);
    deployer.deploy(ProfitableToken);    
};