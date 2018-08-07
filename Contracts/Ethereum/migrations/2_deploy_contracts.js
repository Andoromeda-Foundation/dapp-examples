var Counter = artifacts.require("./Counter.sol");
var PowhToken = artifacts.require("./PowhToken.sol");

module.exports = function(deployer) {
    deployer.deploy(Counter);
    deployer.deploy(PowhToken);    
};