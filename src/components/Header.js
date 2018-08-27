import React, { Component } from "react";
import { NavLink } from 'react-router-dom';
import intl from "react-intl-universal";
import { Layout, Menu, Icon, Row, Col, Avatar, Tooltip, notification } from "antd";
import "nasa.js";
import { NasTool } from "../api/tool";
import axios from "axios";
// import blockies from "ethereum-blockies-png";
import NasId from "../api/nasid";
import BrandLight from "../Brand-Light1.svg";
import BrandDark from "../Brand-Dark1.svg";
import contractoption from "../api/contractoption.js"

// const contract = contractoption.lost_in_nebulas.address;

const { Header } = Layout;
const { SubMenu } = Menu

const getBalance = async (address) => {
    const { data } = await axios.post('https://testnet.nebulas.io/v1/user/accountstate', {
        address
    })
    return data.result.balance
}

const MenuItem = ({ path, name, icon }) =>
    <Menu.Item key={path}>
        <NavLink to={path}>
            {icon ? <Icon type={icon} /> : <div />}
            <span>{name}</span>
        </NavLink>
    </Menu.Item>

function smartNavbarColor({ location, theme }) {
    const isHomePage = location.pathname === '/'
    console.info(`${location.pathname} isHomePage: ${isHomePage}`)
    const navbarThemeColor = theme === 'light' ? "#FFF" : "#000"
    const isTransparent = isHomePage === true && theme !== 'light'
    const headerBackgroundColor = isTransparent ? "rgba(0, 0, 0, 0.47)" : navbarThemeColor
    const otherColor = isTransparent ? "transparent" : headerBackgroundColor
    return { headerBackgroundColor, isTransparent, otherColor }
}

const langList = {
    "zh-CN": "中文",
    "en-US": "English",
    "ja-JP": "日本語",
    "ko-KR": "한국말"
}

const navbarI18n = (name) => intl.get(`navbar.${name}`)

class HeaderComponent extends Component {
    constructor() {
        super()
        // initial state
        this.state = {
            avatar: {
                icon: 'user'
            },
            nickname: null,
            balance: 0
        }
    }

    async componentDidMount() {
        const { login } = this.props
        let addr = '';
        try {
            addr = await window.Nasa.user.getAddr()
            login(addr)
        } catch (error) {
            notification.error({
                message: '无法检查你的钱包地址',
                description: '请确保你安装了浏览器插件或使用 Nano App打开',
                duration: null
            })
            return; // stop running down
        }
        // 与星云api交互 
        try {
            let balance = await getBalance(addr)
            balance = NasTool.fromWeiToNas(balance).toFixed(2).toString(10)
            this.setState({ balance })
        } catch (error) {
            notification.warning({
                message: '无法获取你的钱包余额',
                description: '星云服务器又玩访问延时了，暂时无法知晓你的钱包余额',
                duration: 10
            })
        }
        try {
            // 获取头像
            NasId(addr).then(resp => {
                let avatar = {}
                avatar.src = resp.avatar
                let { nickname } = resp
                this.setState({ avatar, nickname })
            })
        } catch (error) {
            // do nothing.
            // 无视头像获取失败，防止页面crash
        }
    }

    render() {
        const { location, lang, setLanguage, theme, setTheme, account } = this.props
        const { balance, avatar, nickname } = this.state
        const navigationMenus = [
            {
                path: '/',
                icon: 'home',
                name: navbarI18n('home')
            },
            {
                path: '/faq',
                icon: 'area-chart',
                name: navbarI18n('faq')
            },
            {
                path: '/Eth',
                icon: 'area-chart',
                name: navbarI18n("eth")
            }
        ]
        const { headerBackgroundColor, otherColor } = smartNavbarColor({ location, theme })
        // headerBackgroundColor = isHomePage === true ?  : headerBackgroundColor 
        const Brand = theme === 'light' ? BrandDark : BrandLight


        return (<Header className="header" style={{ background: headerBackgroundColor, padding: 0 }}>
            <Row style={
                {
                    background: otherColor,
                }}>
                <Col xxl={4} xl={5} lg={3} sm={24} xs={24}>
                    <div className="logo" >
                        <img src={Brand} alt="Dasdaq Brand"
                            style={{ maxHeight: '3rem' }}></img>
                    </div>
                </Col>
                <Col xxl={20} xl={19} lg={19} sm={24} xs={24}>
                    <Menu
                        theme={theme}
                        mode="horizontal"
                        defaultSelectedKeys={['/']}
                        selectedKeys={[location.pathname]}
                        style={
                            {
                                lineHeight: '64px',
                                background: otherColor,
                                borderBottomColor: otherColor,
                            }}>
                        {
                            navigationMenus.map(MenuItem)
                        }


                        <SubMenu
                            style={{ float: 'right' }}
                            title={<span> {theme} </span>}>
                            <Menu.Item onClick={() => setTheme('SWITCH_TO_DARK')}> DARK </Menu.Item>
                            <Menu.Item onClick={() => setTheme('SWITCH_TO_LIGHT')}> LIGHT </Menu.Item>
                        </SubMenu>
                        <SubMenu
                            style={{ float: 'right' }}
                            title={<span><Icon type="global" /><span> {langList[lang]} </span></span>}>
                            <Menu.Item onClick={() => setLanguage('SWITCH_TO_CHINESE')}>中文</Menu.Item>
                            <Menu.Item onClick={() => setLanguage('SWITCH_TO_ENGLISH')}>English</Menu.Item>
                            <Menu.Item onClick={() => setLanguage('SWITCH_TO_JAPANESE')}>日本語</Menu.Item>
                            <Menu.Item onClick={() => setLanguage('SWITCH_TO_KOREAN')}>한국말</Menu.Item>
                        </SubMenu>
                        {
                            account &&
                            <Menu.Item style={{ float: 'right' }}>
                                <Tooltip placement="bottom" title={
                                    <div>
                                    <div>钱包地址 $ {account}</div>
                                    </div>}>
                                    <Avatar size="large"
                                        {...avatar} />
                                    <span> {balance} NAS</span>
                                    <span> {nickname} </span>
                                </Tooltip>
                            </Menu.Item>
                        }
                    </Menu>
                </Col>
            </Row>
        </Header>)
    }
}


export default HeaderComponent