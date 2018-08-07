pragma solidity ^0.4.24;

import "truffle/Assert.sol";
import "truffle/DeployedAddresses.sol";
import "../contracts/PowhToken.sol";

contract TestPowhToken {
    function test1() public {
        PowhToken powhToken = new PowhToken();
        uint256 ethers = 123456;
        uint256 tokens = powhToken.ethereumToTokens_(ethers);
        Assert.equal(powhToken.tokensToEthereum_(tokens), ethers, "buy and sell should be consistense");
    }
    function test2() public {
        PowhToken powhToken = new PowhToken();
        uint256 ethers = 123456;
        uint256 tokens = powhToken.ethereumToTokens2_(ethers);
        Assert.equal(powhToken.tokensToEthereum_(tokens), ethers, "buy and sell should be consistense");
    }
}
