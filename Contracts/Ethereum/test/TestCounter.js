var Counter = artifacts.require("Counter");

contract('Counter', function(accounts) {
    it("should equal to 0 at the beginning", function() {
        return Counter.deployed().then(function(instance) {
            return instance.get.call();
        }).then(function(counter) {
            assert.equal(counter.valueOf(), 0, "not equal to 0 at the beginning");
        });
    });   
    it("should equal to x after x inc() operations", function() {
        var instance;
        return Counter.deployed().then(function(_instance) {
            instance = _instance;            
            return instance.inc();
          }).then(function() {
            return instance.inc();
          }).then(function() {
            return instance.get.call();
          }).then(function(counter) {
            assert.equal(counter.toNumber(), 2, "should equal to 2 after 2 inc() operations which is" + counter.valueOf());
        })
    });
});