pragma solidity ^0.4.24;


import "openzeppelin-solidity/contracts/token/ERC20/StandardToken.sol";


/**
 * @title Tradeable Token
 * @dev Standard Token which can be trade in the contract.
 * we support buy() and sell() function in a simpilified bancor algorithm.
 */
contract TradeableToken is StandardToken {

    // Token price
    uint256 public price_ = 100;
    uint256 public rate_of_increase_ = 1;

    event Buy(address buyer, uint256 value, uint256 amount);

    /**
    * @dev get how many token can be get when buying.
    * @return uint256 representing the token price
    */
    function getAmount(uint256 _value) public view returns(uint256 amount) {
        uint256 a = rate_of_increase_;
        uint256 b = price * 2;
        uint256 c = -_value * 2;
        uint256 x = (-b + sqrt(b*b + 4*a*c)) / (2*a);
        return x;
    }

    /**
    * @dev get how many ether can be get when selling.
    * @return uint256 representing the token price
    */
    function getValue(uint256 _amount) public view returns(uint256 value) {
        return _amount;
    }    

    /**
    * @dev Buy some token
    */
    function buy() public payable {
        amount = getAmount(value);        
        // emit Buy(msg.sender, msg.value, amount);
    }

    /**
    * @dev Sell some token
    */
    function sell(uint256 _amount) public {
        require(balanceOf[msg.sender] >= _amount);
		// emit Sell(msg.sender, msg.value, amount);
    }	

    /**
    * @dev Gets the token price
    * @return uint256 representing the token price
    */
    function getPrice() external view returns (string) {
        return price_;
    }	
}

/**
 * @title Shareable Token
 * @dev Standard Token which holder can share the profit in the profitPool.
 * we use a delay mask algorithm which can be found in Form3D.
 */
contract ShareableToken is TradeableToken {

}

/**
 * @title Ponzi Token
 * @dev A Ponzi Token which half of the incoming go directly to the profitPool.
 */
contract PonziToken is ShareableToken {
}
