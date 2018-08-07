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
        uint256 tokensMinted
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

    function _mint(address _customerAddress, uint256 _amount) internal {
        require(_amount > 0 && (SafeMath.add(_amount, totalSupply_) > totalSupply_));        
        totalSupply_ = SafeMath.add(totalSupply_, _amount);
        balances[_customerAddress] = SafeMath.add(balances[_customerAddress], _amount);        
    }

    function _burn(address _customerAddress, uint256 _amount) internal {
        require(_amount > 0 && (SafeMath.sub(_amount, totalSupply_) < totalSupply_));            
        totalSupply_ = SafeMath.sub(totalSupply_, _amount);
        balances[_customerAddress] = SafeMath.sub(balances[_customerAddress], _amount);        
    }    
    
    function _buy(uint256 _incomingEther) internal returns(uint256) {
        address _customerAddress = msg.sender;
        uint256 _amountOfTokens = ethereumToTokens_(_incomingEther);
        _mint(_customerAddress, _amountOfTokens);
        emit onBuy(_customerAddress, _incomingEther, _amountOfTokens);        
        return _amountOfTokens;        
    }

    function _sell(uint256 _incomingToken) internal returns(uint256) {
        address _customerAddress = msg.sender;
        require(_incomingToken <= balances[_customerAddress]);
        uint256 _amountOfEther = tokensToEthereum_(_incomingToken);
        _burn(_customerAddress, _incomingToken);
        _customerAddress.transfer(_amountOfEther);
        emit onSell(msg.sender, _incomingToken, _amountOfEther);
        return _amountOfEther;
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
        _sell(_amount);
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
        public
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
                            (((tokenPriceIncremental_)**2)*(totalSupply_**2))
                            +
                            (2*(tokenPriceIncremental_)*_tokenPriceInitial*totalSupply_)
                        )
                    ), _tokenPriceInitial
                )
            )/(tokenPriceIncremental_)
        )-(totalSupply_)
        ;
        require(_tokensReceived == ethereumToTokens2_(_ethereum));
        return _tokensReceived;
    }

    /**
     * Calculate Token price based on an amount of incoming ethereum
     * It's an algorithm, hopefully we gave you the whitepaper with it in scientific notation;
     * Some conversions occurred to prevent decimal errors or underflows / overflows in solidity code.
     */
    function ethereumToTokens2_(uint256 _ethereum)
        public
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
                        )
                    ), _tokenPriceInitial
                )
            )/(tokenPriceIncremental_)
        )
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
