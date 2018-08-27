import { BigNumber } from 'bignumber.js';
import React, { PureComponent } from "react";
import intl from "react-intl-universal";
import "nasa.js";
// import styles from './timing.less';
import { NasTool } from "../api/tool";
import moment from 'moment'
import { Button, Input, Table, Modal, Avatar, Card, Col, Row } from "antd";
import getcontract from "../api/contractbackend.js";
import NasId from "../api/nasid";
import contractoption from "../api/contractoption.js"
var _ = require('lodash');

const backgroundImg = 'https://i.loli.net/2018/07/16/5b4c4a832a920.jpg'
const contract = contractoption.lost_in_nebulas.address;

// var buyList = [
//     { key: "1", player: "猴子", amount: "100", price: "20", time: "2018/7/24 下午10:32:45" },
// ]

const bannerStyle = {
    padding: `6rem`,
    color: `#fafafa`,
    width: "100%", minHeight: "48rem",
    background: `url(${backgroundImg})`, backgroundSize: 'cover'
}

const buttonStyle = {
    margin: "0.5rem"
}

const lableStyle = {
    marginLeft: "0.5rem",
    color: "#000",
    float: "left"
}
const timingStyle = {
    color: '#fff',
    fontSize: '3rem'
}

const colStyle = {
    padding: '0 10px'
}
const K = new BigNumber(NasTool.fromNasToWei(0.000000001).toString())
        
// const titleStyle = {
//     opacity: 1, transform: `translate(0px, 0px)`,
//     fontSize: `68px`,
//     color: `#fafafa`,
//     fontWeight: 600,
//     lineHeight: `76px`,
//     margin: `8px 0 28px`,
//     letterSpacing: 0
// }

function ClaimEvent(e) {
    var args = []
    var option = {}
    window.Nasa.call(contract, "claim", args, option)
        .then((payId) => {
            setTimeout(() => {
            }, 5000)
        })
        .catch((e) => {
            let msg = e.message
            if (msg === window.Nasa.error.TX_REJECTED_BY_USER) {
                msg = intl.get("homepage.tx_rejected_msg");
            }
            alert(msg)
        })
}

class BuyPopup extends React.Component {
    constructor(props) {
        super(props)
        this.state = {
            gas: null,
            nas: null,
            supply: null,
            nas_amount: null,
            gas_amount: null,
            current_price: this.props.current_price,
            totalSupply: null,
            my_token: null
        }
        this.gasToNas = this.gasToNas.bind(this);
        this.nasToGas = this.nasToGas.bind(this);
    }
    componentWillReceiveProps(nextProps) {
        this.setState({ current_price: nextProps.current_price })
        this.setState({ my_token: nextProps.player_balance})
    }
    // async componentDidMount(){
    //     const supply = await window.Nasa.query(contract, "totalSupply", [])
    //     this.setState({supply});
    // }
    BuyEvent(e) {
        var args = []
        var option = {
            "value": document.getElementById("buy_amount").value
        }
        window.Nasa.call(contract, "buy", args, option)
            .then((payId) => {
                setTimeout(() => {
                }, 5000)
            })
            .catch((e) => {
                let msg = e.message
                if (msg === window.Nasa.error.TX_REJECTED_BY_USER) {
                    msg = intl.get("homepage.tx_rejected_msg");
                }
                alert(msg)
            })
    }

    gasToNas(e) {
        let gas_amount = e.target.value
        let nas_amount = ""
        if (gas_amount !== "") {
            let current_price = NasTool.fromNasToWei(new BigNumber(this.state.current_price))
            let wei = (current_price.multipliedBy(2).plus(K.multipliedBy(gas_amount))).multipliedBy(gas_amount).dividedBy(2)
            nas_amount = NasTool.fromWeiToNas(wei)
        }
        this.setState({ gas_amount, nas_amount })
    }

    nasToGas(e) {
        let nas_amount = e.target.value
        let gas_amount = ""
        if (nas_amount !== "") {
            let price = NasTool.fromNasToWei(new BigNumber(this.state.current_price))
            let value = new BigNumber(nas_amount)
            let wei = NasTool.fromNasToWei(value)
            let a = K;
            let b = (new BigNumber(price)).multipliedBy(2);
            let c = (new BigNumber(0)).minus(wei.multipliedBy(2));
            gas_amount = (new BigNumber(0)).minus(b).plus(Math.floor(Math.sqrt(b.multipliedBy(b).minus(a.multipliedBy(c).multipliedBy(4))))).dividedBy((a.multipliedBy(2)));
        }
        this.setState({ gas_amount, nas_amount })
    }

    render() {
        return (
            <Modal
                title={this.props.text}
                visible={this.props.visible}
                onOk={this.BuyEvent}
                onCancel={this.props.close_popup}>
                <div style={lableStyle}>Nas:</div>
                <Input
                    {...this.props}
                    id="buy_amount"
                    placeholder="Input a amount in NAS"
                    maxLength="25"
                    value={this.state.nas_amount}
                    onChange={this.nasToGas}
                />
                <div style={lableStyle}>Gas:</div>
                <Input
                    id="gas"
                    placeholder="Input a amount in gas"
                    onChange={this.gasToNas}
                    maxLength="25"
                    value={this.state.gas_amount}
                />
            </Modal>
        );
    }
}

class SellPopup extends React.Component {
    constructor(props) {
        super(props)
        this.state = {
            gas: null,
            nas: null,
            supply: null,
            sell_amount: null,
            sell_gas: null,
            current_price: this.props.current_price
        }
        this.gasToNas = this.gasToNas.bind(this);
        this.nasToGas = this.nasToGas.bind(this);
    }
    componentWillReceiveProps(nextProps) {
        this.setState({ current_price: nextProps.current_price })
    }
    SellEvent(e) {
        var args = [document.getElementById("sell_gas").value]
        var option = {}

        window.Nasa.call(contract, "sell", args, option)
            .then((payId) => {
                setTimeout(() => {
                }, 5000)
            })
            .catch((e) => {
                let msg = e.message
                if (msg === window.Nasa.error.TX_REJECTED_BY_USER) {
                    msg = intl.get("homepage.tx_rejected_msg");
                }
                alert(msg)
            })
    }
    gasToNas(e) {
        let sell_gas = e.target.value
        let sell_amount = ""
        if (sell_gas !== "") {
            let current_price = NasTool.fromNasToWei(new BigNumber(this.state.current_price))
            let wei = (current_price.multipliedBy(2).minus(K.multipliedBy(sell_gas))).multipliedBy(sell_gas).dividedBy(2)
            sell_amount = NasTool.fromWeiToNas(wei.dividedBy(2))
        }
        this.setState({ sell_gas, sell_amount })
    }
    nasToGas(e) {
        let sell_amount = e.target.value
        let sell_gas = ""
        if (sell_amount !== "") {
            let price = NasTool.fromNasToWei(new BigNumber(this.state.current_price))
            let value = new BigNumber(sell_amount).multipliedBy(2)
            let wei = NasTool.fromNasToWei(value)
            let a = K;
            let b = (new BigNumber(0).minus(price)).multipliedBy(2);
            let c = (new BigNumber(wei).multipliedBy(2));
            sell_gas = (new BigNumber(0)).minus(b).minus(Math.floor(Math.sqrt(b.multipliedBy(b).minus(a.multipliedBy(c).multipliedBy(4))))).dividedBy((a.multipliedBy(2)));
        }
        this.setState({ sell_gas, sell_amount })
    }
    render() {
        return (
            <Modal
                title={this.props.text}
                visible={this.props.visible}
                onOk={this.SellEvent}
                onCancel={this.props.close_popup}>
                <div style={lableStyle}>Nas:</div>
                <Input
                    {...this.props}
                    id="sell_amount"
                    placeholder="Input a amount in Token"
                    maxLength="25"
                    onChange={this.nasToGas}
                    value={this.state.sell_amount}
                />
                <div style={lableStyle}>
                    Gas:
                    </div>
                <Input
                    id="sell_gas"
                    onChange={this.gasToNas}
                    value={this.state.sell_gas}
                    placeholder="Input a amount in gas"
                    maxLength="25" />
            </Modal>
        );
    }
}

class Timing extends PureComponent {
    state = {
        endTime: '2018/07/28 15:39:10',
        day: '0',
        hour: '00',
        minute: '00',
        second: '00',
    }
    componentWillMount() {
        this.timer = setInterval(
            () => {
                let loss = '';
                const M = moment().format('MM');
                const D = moment().format('DD');
                const Y = moment().format('YYYY');
                const S = moment().format('HH:mm:ss');
                const now = `${Y}/${M}/${D} ${S}`;
                const pre = `${Y}/${M}/${D} 23:59:59`;
                // loss = parseInt((new Date(this.state.endTime) - new Date(now)) / 1000, 10);
                loss = parseInt((new Date(pre) - new Date(now)) / 1000, 10);
                if (loss < 0) {
                    this.setState({
                        day: '0',
                        hour: '00',
                        minute: '00',
                        second: '00',
                    });
                    clearInterval(this.timer);
                    return;
                }
                const day = parseInt((loss * 1) / 86400, 0);
                loss = (loss * 1) % 86400;
                let hour = parseInt((loss * 1) / 3600, 0);
                loss = (loss * 1) % 3600;
                let minute = parseInt((loss * 1) / 60, 0);
                let second = (loss * 1) % 60;
                if (hour * 1 < 10) {
                    hour = `0${hour}`;
                }
                if (minute * 1 < 10) {
                    minute = `0${minute}`;
                }
                if (second * 1 < 10) {
                    second = `0${second}`;
                }
                this.setState({
                    day,
                    hour,
                    minute,
                    second,
                });
            },
            1000
        );
    }
    render() {
        const { state } = this;
        return (
            <div>
                <h1 style={timingStyle}>{state.hour}:{state.minute}:{state.second}</h1>
            </div>
        );
    }
}

class Home extends React.Component {
    constructor() {
        super();
        this.state = {
            showPopup: false,
            buyList: null,
            current_price: null, // 当前合约代币价格
            bonus_balance: null, // 大奖奖池
            share_balance: null, // 总分红池
            player_balance: null, // 用户持有的代币数量
            // 用户分红信息
            player_available_share: null,
            player_claimed_share: null,
            player_total_share: null,
            // 当前总供给
            total_supply: null
        };
    }

    async fetchPriceAndBalance() {
        const { account } = this.props
        // 价格
        const current_price = NasTool.fromWeiToNas(await window.Nasa.query(contract, "getPrice", [])).toString()

        // 合约池信息
        // 最终大奖奖池
        const bonus_balance = NasTool.fromWeiToNas(await window.Nasa.query(contract, "getBonusPool", [])).toString()
        // 分红池
        const share_balance = NasTool.fromWeiToNas(await window.Nasa.query(contract, "getProfitPool", [])).toString()

        // 玩家信息
        // 玩家token持有量
        const player_balance = (await window.Nasa.query(contract, "balanceOf", [account])).toString()
        // 等待领取的分红数量 
        const player_available_share = NasTool.fromWeiToNas(await window.Nasa.query(contract, "getAvailableShare", [account])).toString()
        // 已经领取的分红数量
        const player_claimed_share = NasTool.fromWeiToNas(await window.Nasa.query(contract, "getClaimedProfit", [account])).toString()
        // 用户得到的总分红数量
        const player_total_share = new BigNumber(player_available_share).plus(player_claimed_share).toString()
        
        // 当前总卖出的token
        const total_supply = (await window.Nasa.query(contract, "totalSupply", [])).toString()

        return { current_price, bonus_balance, share_balance, player_balance, player_available_share, player_claimed_share, player_total_share, total_supply }
    }

    async getList() {
        try {
            return await getcontract(contract)
        } catch (e) {
            return []
        }
    }

    getnasid(list) {
        var nasidlist = []; //缓存，地址相同即读取此
        _.each(list, async (one, index) => {
            await new Promise((resolve, reject) => {
                _.each(nasidlist, (oneoflist) => {
                    if (one.player == oneoflist.player) {
                        var buyList = this.state.buyList
                        buyList[index].nickname = oneoflist.nickname;
                        buyList[index].avatar = oneoflist.avatar;
                        this.setState({ buyList })
                        resolve();
                    }
                })
                NasId(one.player).then(resp => {
                    let avatar = {}
                    avatar.player = one.player
                    if (resp != undefined) {
                        avatar.src = resp.avatar
                        avatar.nickname = resp.nickname
                    }
                    nasidlist.push(avatar)
                    var buyList = this.state.buyList
                    buyList[index].nickname = avatar.nickname;
                    buyList[index].avatar = avatar.src;
                    this.setState({ buyList })
                    resolve();
                }).catch(e => { }) // avoid page crash
            })
        })
    }

    componentWillMount() {
        window.Nasa.env.set("testnet")
        window.Nasa.contract.set({
            default: {
                local: contract,
                testnet: contract,
                mainnet: contract,
            }
        })
    }

    async componentDidMount() {
        const {
            current_price,
            bonus_balance,
            share_balance,
            player_balance,
            player_available_share,
            player_claimed_share,
            player_total_share,
            total_supply
        } = await this.fetchPriceAndBalance()
        const buyList = await this.getList()
        this.setState({
            current_price,
            bonus_balance,
            share_balance,
            player_balance,
            player_available_share,
            player_claimed_share,
            player_total_share,
            total_supply
        })
        this.setState({ buyList })
        this.getnasid(buyList);
    }

    toggleBuyPopup() {
        this.setState({
            showBuyPopup: !this.state.showBuyPopup
        });
    }

    toggleSellPopup() {
        this.setState({
            showSellPopup: !this.state.showSellPopup
        });
    }

    render() {
        const { account } = this.props
        const {
            current_price,
            bonus_balance,
            share_balance,
            player_balance,
            player_available_share,
            player_claimed_share,
            player_total_share,
            total_supply,
            buyList
        } = this.state
        const columns = [{
            title: intl.get("history.player"),
            dataIndex: 'player',
            key: 'player',
            render: (text, record) => (
                <span>
                    <Avatar size="large" src={record.avatar} />
                    <span> {record.nickname} </span>
                </span>
            ),
        }, {
            title: intl.get("history.event"),
            dataIndex: 'event',
            key: 'event',
            defaultSortOrder: 'descend',
            sorter: (a, b) => a.event.length - b.event.length,
        }, {
            title: intl.get("history.amount"),
            dataIndex: 'amount',
            key: 'amount',
            defaultSortOrder: 'descend',
            sorter: (a, b) => parseInt(a.amount, 10) - parseInt(b.amount, 10),
        }, {
            title: intl.get("history.price"),
            dataIndex: 'price',
            key: 'price',
            defaultSortOrder: 'descend',
            sorter: (a, b) => parseInt(a.price, 10) - parseInt(b.price, 10),
        }, {
            title: intl.get("history.time"),
            dataIndex: 'time',
            key: 'time',
            sortOrder: 'descend',
            sorter: (a, b) => parseInt(a.timesecond, 10) - parseInt(b.timesecond, 10),
        }];

        return (
            <div className="index-page" style={{ marginTop: "-64px" }}>
                <div className="banner" style={bannerStyle}>
                    <Row>
                {/*        <Col span="5" style={colStyle}>
                            <Card bordered={false}>
                                <div className="custom-image" style={{ marginBottom: '5px' }}>
                                    {intl.get("homepage.player_balance")}
                                </div>
                                <div className="custom-card">
                                    {player_balance?(player_balance.substr(0,player_balance.length>15?15:player_balance.length)):0} Gas
                                    </div>
                            </Card>
                        </Col>*/}
                        <Col span="6" style={colStyle}>
                            <Card bordered={false}>
                                <div className="custom-image" style={{ marginBottom: '5px' }}>
                                    {intl.get("homepage.contract_claim_balance")}
                                </div>
                                <div className="custom-card">
                                    {share_balance?(share_balance.substr(0,share_balance.length>15?15:share_balance.length)):0} NAS
                                    </div>
                            </Card>
                        </Col>
                        <Col span="6" style={colStyle}>
                            <Card bordered={false}>
                                <div className="custom-image" style={{ marginBottom: '5px' }}>
                                    {intl.get("homepage.contract_bonus_balance")}
                                </div>
                                <div className="custom-card">
                                    {bonus_balance?(bonus_balance.substr(0,bonus_balance.length>15?15:bonus_balance.length)):0} NAS
                                    </div>
                            </Card>
                        </Col>
                        <Col span="6" style={colStyle}>
                            <Card bordered={false}>
                                <div className="custom-image" style={{ marginBottom: '5px' }}>
                                    {intl.get("homepage.my_claim_balance")}
                                </div>
                               {/* <div className="custom-card">
                                    {player_available_share}({player_total_share}) NAS
                                    </div>*/}
                                <div className="custom-card">
                                    {player_available_share?(player_available_share.substr(0,player_available_share.length>15?15:player_available_share.length)):0}NAS
                                </div>
                            </Card>
                        </Col>
                        <Col span="6" style={colStyle}>
                            <Card bordered={false}>
                                <div className="custom-image" style={{ marginBottom: '5px' }}>
                                    {intl.get("homepage.current_price")}
                                </div>
                                <div className="custom-card">
                                    {current_price?(current_price.substr(0,current_price.length>15?15:current_price.length)):0}NAS
                                    </div>
                            </Card>
                        </Col>

                    </Row>
                    <div style={{paddingTop:'10px'}}>
                    <Row>
                        <Col span="12" style={colStyle}>
                            <Card bordered={false}>
                                <div className="custom-image" style={{ marginBottom: '5px' }}>
                                    {intl.get("homepage.my_token")}
                                </div>
                                <div className="custom-card">
                                    {player_balance?(player_balance.substr(0,player_balance.length>15?15:player_balance.length)):0}Gas
                                </div>
                            </Card>
                        </Col>
                        <Col span="12" style={colStyle}>
                            <Card bordered={false}>
                                <div className="custom-image" style={{ marginBottom: '5px' }}>
                                    {intl.get("homepage.totalSupply")}
                                </div>
                                <div className="custom-card">
                                    {total_supply?(total_supply.substr(0,total_supply.length>15?15:total_supply.length)):0}Gas
                                </div>
                            </Card>
                        </Col>
                    </Row>
                    </div>
                    {/*  <div> {intl.get("homepage.contract_balance")}: {current_balance} NAS</div>
                    <div> {intl.get("homepage.contract_claim_balance")}: {claim_balance} NAS</div>
                    <div> {intl.get("homepage.contract_bonus_balance")}: {bonus_balance} NAS</div>
                    <div> {intl.get("homepage.my_claim_balance")}: {my_claim_balance} NAS</div>
                    <div> {intl.get("homepage.user_addr")}: {account} </div>*/}
                    {/*<div style={priceStyle}> {intl.get("homepage.current_price")}: {current_price} NAS</div>*/}
                    <Timing />
                    <Button type="primary" size="large" style={buttonStyle} onClick={this.toggleBuyPopup.bind(this)}>
                        {intl.get('homepage.buy_button')}
                    </Button>
                    <Button type="primary" size="large" style={buttonStyle} onClick={this.toggleSellPopup.bind(this)}>
                        {intl.get('homepage.sell_button')}
                    </Button>
                    <Button type="primary" size="large" style={buttonStyle} onClick={ClaimEvent}>
                        {intl.get('homepage.claim_button')}
                    </Button>
                    {this.state.showBuyPopup ?
                        <BuyPopup
                            text={intl.get('homepage.buy_title')}
                            visible={this.state.showBuyPopup}
                            close_popup={this.toggleBuyPopup.bind(this)}
                            current_price={current_price}
                        />
                        : null
                    }
                    {this.state.showSellPopup ?
                        <SellPopup
                            text={intl.get('homepage.sell_title')}
                            visible={this.state.showSellPopup}
                            close_popup={this.toggleSellPopup.bind(this)}
                            current_price={current_price}
                        />
                        : null
                    }

                    <div>
                        游戏背景：支援获得了价值连城的帝国宝物的反抗军安全返回地球！反抗军首领将会把宝藏分给最后支援的人。
                    </div><div>
                        游戏规则：每购买至少 1 单位 gas 燃料，反抗军就可以再多周旋 24 小时。宝藏的价值也会增加。
                    <div>gas 燃料价格等于: basePrice + k x supply</div>
                    </div>
                    <Table 
                        dataSource={buyList} 
                        columns={columns}                     
                        sortField="time"
                        sortOrder="descend"                    
                       style={{ background: `white` }} />
                </div>
            </div>
        );
    }
};

export default Home;