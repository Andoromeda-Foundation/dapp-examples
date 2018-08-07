var TradeableToken = artifacts.require("TradeableToken");

contract('TradeableToken', function(accounts) {
    it("should be almost equal after buy and sell", async () => {

        let tradeableToken = await TradeableToken.deployed();

        let contractEther = await web3.eth.getBalance(tradeableToken.address).toNumber();        

        let beginEther = await web3.eth.getBalance(accounts[0]).toNumber();
        let usedEther = beginEther / 2;
        let unusedEther = beginEther - usedEther;

        await tradeableToken.buy({from: accounts[0], value: usedEther});
        let tokenBalance = await tradeableToken.balanceOf.call(accounts[0]);
        // tokenBalance += 1;

        await tradeableToken.sell(tokenBalance, {from: accounts[0]});        
        let finalEther = await web3.eth.getBalance(accounts[0]).toNumber();
        let refundEther = finalEther - unusedEther;
        let consumingEther = usedEther - refundEther;

        let contractEther2 = await web3.eth.getBalance(tradeableToken.address).toNumber();        
    //    console.log(tradeableToken);

        console.log(beginEther);
        console.log(usedEther);
        console.log(refundEther);        
        console.log(unusedEther);        
        console.log(finalEther);
        console.log(consumingEther);

        console.log(contractEther);
        console.log(contractEther2);

        assert.isAtMost(refundEther, usedEther, "Amount should be almost equal");      
    });
});