pragma solidity ^0.4.24;

import "openzeppelin-solidity/contracts/token/ERC20/StandardToken.sol";
import "openzeppelin-solidity/contracts/math/SafeMath.sol";

/**
 * @title Tradeable Token
 * @dev Standard Token which can be trade within the contract.
 * we support buy() and sell() function in a simpilified bancor algorithm which CW = 50%.
 */
contract TradeableToken is StandardToken {

    // Token price
    string public name = "Dgame Maker";
    string public symbol = "DGM";
    uint8 constant public decimals = 18;
    uint8 constant internal dividendFee_ = 10;
    uint256 constant internal tokenPriceInitial_ = 0.0000001 ether;
    uint256 constant internal tokenPriceIncremental_ = 0.00000001 ether;
    uint256 constant internal magnitude = 2**64;

    /*==========================================
    =                 EVENT                    =
    ==========================================*/   
    event onBuy(
        address indexed customerAddress,
        uint256 incomingEthereum,
        uint256 tokensMinted,
        address indexed referredBy
    );
    
    event onSell(
        address indexed customerAddress,
        uint256 tokensBurned,
        uint256 ethereumEarned
    );
    
    event onWithdraw(
        address indexed customerAddress,
        uint256 ethereumWithdrawn
    );

    /*==========================================
    =            INTERNAL FUNCTIONS            =
    ==========================================*/
    
    function _buy(uint256 _incomingEther) internal {
    }

    /*==========================================
    =            PUBLIC FUNCTIONS              =
    ==========================================*/        
    
    /**
    * @dev get how many token can be get when buying.
    * @return uint256 representing the token price
    */
    function getAmount(uint256 _value) public view returns(uint256 amount) {
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
        _buy(msg.value);
    }

    /**
    * @dev Sell some token
    */
    function sell(uint256 _amount) public {
        // require(balanceOf[msg.sender] >= _amount);
		// emit Sell(msg.sender, msg.value, amount);
    }	

    /**
    * @dev Gets the token price
    * @return uint256 representing the token price
    */
    function getPrice() external view returns (uint256) {
        return 0;
    }

    /**
     * Calculate Token price based on an amount of incoming ethereum
     * It's an algorithm, hopefully we gave you the whitepaper with it in scientific notation;
     * Some conversions occurred to prevent decimal errors or underflows / overflows in solidity code.
     */
    function ethereumToTokens_(uint256 _ethereum)
        internal
        view
        returns(uint256)
    {
        uint256 _tokenPriceInitial = tokenPriceInitial_ * 1e18;
        uint256 _tokensReceived = 
         (
            (
                // underflow attempts BTFO
                SafeMath.sub(
                    (sqrt
                        (
                            (_tokenPriceInitial**2)
                            +
                            (2*(tokenPriceIncremental_ * 1e18)*(_ethereum * 1e18))
                            +
                            (((tokenPriceIncremental_)**2)*(totalSupply()**2))
                            +
                            (2*(tokenPriceIncremental_)*_tokenPriceInitial*totalSupply())
                        )
                    ), _tokenPriceInitial
                )
            )/(tokenPriceIncremental_)
        )-(totalSupply())
        ;
  
        return _tokensReceived;
    }
    
    /**
     * Calculate token sell value.
     * It's an algorithm, hopefully we gave you the whitepaper with it in scientific notation;
     * Some conversions occurred to prevent decimal errors or underflows / overflows in solidity code.
     */
     function tokensToEthereum_(uint256 _tokens)
        internal
        view
        returns(uint256)
    {

        uint256 tokens_ = (_tokens + 1e18);
        uint256 totalSupply_ = (totalSupply() + 1e18);
        uint256 _etherReceived =
        (
            // underflow attempts BTFO
            SafeMath.sub(
                (
                    (
                        (
                            tokenPriceInitial_ +(tokenPriceIncremental_ * (totalSupply_/1e18))
                        )-tokenPriceIncremental_
                    )*(tokens_ - 1e18)
                ),(tokenPriceIncremental_*((tokens_**2-tokens_)/1e18))/2
            )
        /1e18);
        return _etherReceived;
    }
       
    // 
    function sqrt(uint x) internal pure returns (uint y) {
        uint z = (x + 1) / 2;
        y = x;
        while (z < y) {
            y = z;
            z = (x / z + z) / 2;
        }
    }    
}
