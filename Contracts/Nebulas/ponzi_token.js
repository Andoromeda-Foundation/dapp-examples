/**
 * CryptoHero Contract Nebulas Version
 * ©️ Andoromeda Foundation All Right Reserved.
 * @author: Frank Wei <frank@frankwei.xyz>
 * @version: 1.0
 */
"use strict"

class Operator {
    constructor(obj) {
        this.operator = {}
        this.parse(obj)
    }

    toString() {
        return JSON.stringify(this.operator)
    }

    parse(obj) {
        if (typeof obj != "undefined") {
            var data = JSON.parse(obj)
            for (var key in data) {
                this.operator[key] = data[key]
            }
        }
    }

    get(key) {
        return this.operator[key]
    }

    set(key, value) {
        this.operator[key] = value
    }
}

class Tool {
    static fromNasToWei(value) {
        return new BigNumber("1000000000000000000").times(value)
    }
    static fromWeiToNas(value) {
        if (value instanceof BigNumber) {
            return value.dividedBy("1000000000000000000")
        } else {
            return new BigNumber(value).dividedBy("1000000000000000000")
        }
    }
}

var Allowed = function (obj) {
    this.allowed = {};
    this.parse(obj);
}

Allowed.prototype = {
    toString: function () {
        return JSON.stringify(this.allowed);
    },

    parse: function (obj) {
        if (typeof obj != "undefined") {
            var data = JSON.parse(obj);
            for (var key in data) {
                this.allowed[key] = new BigNumber(data[key]);
            }
        }
    },

    get: function (key) {
        return this.allowed[key];
    },

    set: function (key, value) {
        this.allowed[key] = new BigNumber(value);
    }
}

var StandardToken = function () {
    LocalContractStorage.defineProperties(this, {
        _name: null,
        _symbol: null,
        _decimals: null,
        _totalSupply: {
            parse: function (value) {
                return new BigNumber(value);
            },
            stringify: function (o) {
                return o.toString(10);
            }
        }
    });

    LocalContractStorage.defineMapProperties(this, {
        "balances": {
            parse: function (value) {
                return new BigNumber(value);
            },
            stringify: function (o) {
                return o.toString(10);
            }
        },
        "allowed": {
            parse: function (value) {
                return new Allowed(value);
            },
            stringify: function (o) {
                return o.toString();
            }
        }
    });
}

StandardToken.prototype = {
    init: function (name, symbol, decimals, totalSupply) {
        this._name = name;
        this._symbol = symbol;
        this._decimals = decimals || 0;
        this._totalSupply = totalSupply;

        var from = Blockchain.transaction.from;
        this.balances.set(from, this._totalSupply);
        this.transferEvent(true, from, from, this._totalSupply);
    },

    // Returns the name of the token
    name: function () {
        return this._name;
    },

    // Returns the symbol of the token
    symbol: function () {
        return this._symbol;
    },

    // Returns the number of decimals the token uses
    decimals: function () {
        return this._decimals;
    },

    totalSupply: function () {
        return this._totalSupply.toString(10);
    },

    balanceOf: function (owner) {
        var balance = this.balances.get(owner);

        if (balance instanceof BigNumber) {
            return balance.toString(10);
        } else {
            return "0";
        }
    },

    transfer: function (to, value) {
        value = new BigNumber(value);
        if (value.lt(0)) {
            throw new Error("invalid value.");
        }

        var from = Blockchain.transaction.from;
        var balance = new BigNumber(this.balances.get(from)) || new BigNumber(0);

        if (balance.lt(value)) {
            throw new Error("transfer failed.");
        }

        this.balances.set(from, balance.sub(value));
        var toBalance = this.balances.get(to) || new BigNumber(0);
        this.balances.set(to, toBalance.add(value));

        this.transferEvent(true, from, to, value);
    },

    transferFrom: function (from, to, value) {
        var spender = Blockchain.transaction.from;
        var balance = new BigNumber(this.balances.get(from)) || new BigNumber(0);

        var allowed = this.allowed.get(from) || new Allowed();
        var allowedValue = new BigNumber(allowed.get(spender)) || new BigNumber(0);
        value = new BigNumber(value);

        if (value.gte(0) && balance.gte(value) && allowedValue.gte(value)) {

            this.balances.set(from, balance.sub(value));

            // update allowed value
            allowed.set(spender, allowedValue.sub(value));
            this.allowed.set(from, allowed);

            var toBalance = this.balances.get(to) || new BigNumber(0);
            this.balances.set(to, toBalance.add(value));

            this.transferEvent(true, from, to, value);
        } else {
            throw new Error("transfer failed.");
        }
    },

    transferEvent: function (status, from, to, value) {
        Event.Trigger(this.name(), {
            Status: status,
            Transfer: {
                from: from,
                to: to,
                value: value
            }
        });
    },

    approve: function (spender, currentValue, value) {
        var from = Blockchain.transaction.from;

        var oldValue = this.allowance(from, spender);
        if (oldValue != currentValue.toString()) {
            throw new Error("current approve value mistake.");
        }

        var balance = new BigNumber(this.balanceOf(from));
        var value = new BigNumber(value);

        if (value.lt(0) || balance.lt(value)) {
            throw new Error("invalid value.");
        }

        var owned = this.allowed.get(from) || new Allowed();
        owned.set(spender, value);

        this.allowed.set(from, owned);

        this.approveEvent(true, from, spender, value);
    },

    approveEvent: function (status, from, spender, value) {
        Event.Trigger(this.name(), {
            Status: status,
            Approve: {
                owner: from,
                spender: spender,
                value: value
            }
        });
    },

    allowance: function (owner, spender) {
        var owned = this.allowed.get(owner);

        if (owned instanceof Allowed) {
            var spender = owned.get(spender);
            if (typeof spender != "undefined") {
                return spender.toString(10);
            }
        }
        return "0";
    }
}

class ShareableToken extends StandardToken {
    constructor() {
        super()
        LocalContractStorage.defineProperties(this, {
            profitPool: null,
            // Profit per token * _totalSupply.
            calculateProfit: null
        })
        LocalContractStorage.defineMapProperties(this, {
            claimedPPT: null,
            claimedTotalProfit: null
        })
    }

    init(name, symbol, decimals, totalSupply) {
        super.init(name, symbol, decimals, totalSupply)
        this.profitPool = new BigNumber(0)
        this.calculateProfit = new BigNumber(0)
    }

    getProfitPool() {
        return new BigNumber(this.profitPool)
    }

    getProfitPerToken() {
        if (new BigNumber(this._totalSupply).gt(0)) {
            return (new BigNumber(this.calculateProfit)).dividedBy(this._totalSupply)
        } else {
            return new BigNumber(0);
        }
    }

    getAvailableShare(from) {
        var current_ppt = this.getProfitPerToken()
        var claimed_ppt = this.claimedPPT.get(from) || new BigNumber(0)
        return current_ppt.sub(claimed_ppt).mul(this.balanceOf(from));
    }

    getMyProfit() {
        var {
            from
        } = Blockchain.transaction
        return this.getAvailableShare(from)
    }

    getClaimedProfit() {
        var {
            from
        } = Blockchain.transaction
        return this.claimedTotalProfit.get(from) || 0
    }

    claimEvent(status, _from, _value) {
        Event.Trigger(this.name(), {
            Status: status,
            Transfer: {
                from: _from,
                value: _value
            }
        })
    }

    claimFrom(from) {
        var current_ppt = this.getProfitPerToken()
        var claimed_ppt = this.claimedPPT.get(from) || new BigNumber(0)
        var delta_share = current_ppt.sub(claimed_ppt).mul(this.balanceOf(from));
        this.claimedPPT.set(from, current_ppt)
        Blockchain.transfer(from, delta_share)
        this.claimedTotalProfit.set(from, new BigNumber(this.claimedTotalProfit.get(from) || 0).add(delta_share))
        this.claimEvent(true, from, delta_share)
    }

    claim() {
        var {
            from
        } = Blockchain.transaction
        this.claimFrom(from)
    }

    transfer(to, value) {
        this.claim()
        this.claimFrom(to)
        super.transfer(to, value)
    }

    transferFrom(from, to, value) {
        this.claimFrom(from)
        this.claimFrom(to)
        super.transferFrom(from, to, value)
    }
}

class OwnerableContract extends ShareableToken {
    constructor() {
        super()
        LocalContractStorage.defineProperties(this, {
            owner: null
        })
        LocalContractStorage.defineMapProperties(this, {
            admins: null
        })
    }
    //name, symbol, decimals, totalSupply

    init() {
        super.init("lost in nebulas", "lnb", "18", "0")
        const {
            from
        } = Blockchain.transaction
        this.admins.set(from, "true")
        this.owner = from
    }

    onlyAdmins() {
        const {
            from
        } = Blockchain.transaction
        if (!this.admins.get(from)) {
            throw new Error("Sorry, You don't have the permission as admins.")
        }
    }

    onlyContractOwner() {
        const {
            from
        } = Blockchain.transaction
        if (this.owner !== from) {
            throw new Error("Sorry, But you don't have the permission as owner.")
        }
    }

    getContractOwner() {
        return this.owner
    }

    getAdmins() {
        return this.admins
    }

    grantAdmins(address) {
        this.onlyContractOwner()
        this.admins.set(address, "true")
    }

    revokeAdmins(address) {
        this.onlyContractOwner()
        this.admins.set(address, "false")
    }

}

const K = Tool.fromNasToWei(0.000000001)
const initialTokenPrice = Tool.fromNasToWei(0.0001)

var Order = function (obj) {
    this.parse(obj);
};

Order.prototype = {
    toString: function () {
        return JSON.stringify(this);
    },

    parse: function (obj) {
        if (typeof obj != "undefined") {
            var data = JSON.parse(obj);
            this.orderId = data.orderId;
            this.account = data.account;
            this.amount = data.amount;
            this.value = data.value;
            this.timestamp = data.timestamp;
            this.type = data.type;
        } else {
            this.orderId = 0;
            this.account = '';
            this.amount = 0;
            this.value = 0;
            this.crr = 0;
            this.timestamp = 0;
            this.type = '';
        }
    }
};

class LostInNebulasContract extends OwnerableContract {
    constructor() {
        super()
        LocalContractStorage.defineProperties(this, {
            price: null,
            referCut: new BigNumber(5),    // 0.2 of share Cut
            lastBuyTime: null,
            shareCut: new BigNumber(2),    // 0.5
            bonusPool: null,
            MAX_TIME: null,
            lastBuyer: null,
            orderIndex: {
                parse: function (value) {
                    return new BigNumber(value);
                },
                stringify: function (o) {
                    return o.toString(10);
                }
            }
        })
        LocalContractStorage.defineMapProperties(this, {
            orderList: {
                parse: function (value) {
                    return [].concat(JSON.parse(value));
                },
                stringify: function (o) {
                    return JSON.stringify(o);
                }
            }
        })
    }

    init() {
        super.init()
        var {
            from
        } = Blockchain.transaction
        this.orderIndex = new BigNumber(0)
        this.price = new BigNumber(initialTokenPrice)
        this.bonusPool = new BigNumber(0)
        this.shareCut = new BigNumber(2) // 0.5
        this.referCut = new BigNumber(5) // 0.2 of shareCut.
        this.MAX_TIME = 86400
        this.lastBuyTime = this._getNow()
    }

    claim() {
        super.claim()
        if (this.isRoundOver() && (from == this.lastBuyer)) {
            Blockchain.transfer(from, new BigNumber(this.bonusPool))
            this.bonusPool = new BigNumber(0)
        }
    }

    getLastBuyer() {
        return this.lastBuyer
    }
    
    getAmountByValue(value) {
        // (2p + kx)x/2 = value
        // kx^2 + 2px - 2value = 0
        var a = K;
        var b = (new BigNumber(this.price)).mul(2);
        var c = (new BigNumber(0)).sub(value.mul(2));
        var x = (new BigNumber(0)).sub(b).add(Math.floor(Math.sqrt(b.mul(b).sub(a.mul(c).mul(4))))).dividedBy((a.mul(2)));

        return x;
    }

    getValueByAmount(amount) {
        // (p + p - k*am)*am /2
        var value = (new BigNumber(this.price)).add(this.price).sub(K.mul(amount).mul(amount).dividedBy(2));
        return value;
    }

    getLastBuyTime() {
        return this.lastBuyTime
    }

    getActiveTime() {
        return new BigNumber(this.lastBuyTime).add(this.MAX_TIME)
    }

    getBonusPool() {
        return this.bonusPool
    }

    getPrice() {
        return this.price
    }

    getOrderIndex() {
        return this._orderIndex.toString(10);
    }

    getOrder(_index) {
        return this.orderList.get(new BigNumber(_index)) || [];
    }

    _getNow() {
        return new BigNumber(Math.floor(Date.parse(new Date()) / 1000))
    }

    updateLastBuyTime(times) {
        this.lastBuyTime = new BigNumber(this.lastBuyTime).add(Math.floor(30 * times))
        if (new BigNumber(this.lastBuyTime).gte(this._getNow())) {
            this.lastBuyTime = this._getNow()
        }
    }

    isRoundOver() {
        let active = new BigNumber(this.lastBuyTime).add(this.MAX_TIME)
        return !active.gte(this._getNow())
    }

    buyEvent(status, _from, _value, _amount) {
        Event.Trigger(this.name(), {
            Status: status,
            Transfer: {
                from: _from,
                value: _value,
                amount: _amount
            }
        })
    }

    sellEvent(status, _from, _amount, _value) {
        Event.Trigger(this.name(), {
            Status: status,
            Transfer: {
                from: _from,
                amount: _amount,
                value: _value
            }
        })
    }

    buy(referal = "") {
        var {
            from,
            value
        } = Blockchain.transaction

        if (this.isRoundOver()) {
            throw Error("Game over")
        }

        this.claim()  // claim share before every buy.

        value = new BigNumber(value)

        if (value.gte(1)) {
            this.lastBuyer = from
        }

        var amount = this.getAmountByValue(value)
        this.updateLastBuyTime(amount)
        var profit = value.dividedBy(this.shareCut)
        this.bonusPool = new BigNumber(this.bonusPool).add(value.sub(profit))  // half to bonus pool
        if (referal !== "") {
            var referal_bonus = profit.dividedBy(this.referCut)
            Blockchain.transfer(referal, referal_bonus)
            profit = profit.sub(referal_bonus)
        }
        this.calculateProfit = new BigNumber(this.calculateProfit).add(this.getProfitPerToken().mul(amount)).plus(profit)
        this.profitPool = (new BigNumber(this.profitPool)).plus(profit)

        this._totalSupply = new BigNumber(this._totalSupply).plus(amount)
        this.balances.set(from, (new BigNumber(this.balances.get(from) || 0)).plus(amount))
        this.price = (new BigNumber(this.price)).add(K.mul(amount))

        var buyOrder = new Order();
        buyOrder.orderId = parseInt(new BigNumber(this.orderIndex).plus(1).toString(10));
        buyOrder.account = from;
        var now = Date.now();
        buyOrder.timestamp = parseInt(now);
        buyOrder.amount = amount
        buyOrder.value = value
        buyOrder.type = "buy";

        this.orderList.put(this.orderIndex, buyOrder);
        this.orderIndex = this.orderIndex.plus(1);
        this.buyEvent(true, from, value, amount)
    }

    sell(amount) {
        var {
            from
        } = Blockchain.transaction

        if (this.isRoundOver()) {
            throw Error("Game over")
        }

        this.claim()

        amount = new BigNumber(amount)
        var value = this.getValueByAmount(amount)
        value = value.sub(value.dividedBy(this.shareCut))  // only return bonus pool

        if (!new BigNumber(this.balances.get(from)).gte(amount)) {
            // Check amount.
            throw Error("amount not enough.")
        }
        this.bonusPool = new BigNumber(this.bonusPool).sub(value)
        this._totalSupply = new BigNumber(this._totalSupply).sub(amount)
        this.balances.set(from, (new BigNumber(this.balances.get(from))).sub(amount))
        this.price = (new BigNumber(this.price)).sub(K.mul(amount))

        var sellOrder = new Order();
        sellOrder.orderId = parseInt(this.orderIndex.plus(1).toString(10));
        sellOrder.account = from;
        var now = Date.now();
        sellOrder.timestamp = parseInt(now);
        sellOrder.amount = amount
        sellOrder.value = value
        sellOrder.type = "sell";

        this.orderList.put(this.orderIndex, sellOrder);
        this.orderIndex = this.orderIndex.plus(1);
        this.sellEvent(true, from, amount, value)
        Blockchain.transfer(from, value)
    }
}

module.exports = LostInNebulasContract