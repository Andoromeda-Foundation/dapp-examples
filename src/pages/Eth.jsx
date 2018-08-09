import React from "react";

import Web3 from 'web3'; 
import { Button , Col} from "antd";
import MyCounterAbi from "../api/MycounterAbi";

const backgroundImg = 'https://i.loli.net/2018/07/16/5b4c4a832a920.jpg'
const contractAddr = 'https://explorer.nebulas.io/#/testnet/address/n1vhZgBFYt7AE6nP3VFap9c67VPqn1eFoTi'
const buttonStyle = {
    margin: "0.5rem"
}
const colStyle = {
    padding: '0 10px'
}
const bannerStyle = {
    padding: `6rem`,
    color: `#fafafa`,
    width: "100%", minHeight: "48rem",
    background: `url(${backgroundImg})`, backgroundSize: 'cover'
}
const number = {
    width: '150px',
    height: '150px',
    backgroundColor: 'darksalmon'
}


// window.addEventListener('load', 
//   function () {
//     if (typeof web3 !== 'undefined') {        
//         window.web3 = new Web3(window.web3.currentProvider)
//         if (window.web3.currentProvider.isMetaMask === true) {
//             window.web3.eth.getAccounts((error, accounts) => {
//                 if (accounts.length == 0) {
//                     alert("there is no active accounts in MetaMask")
//         
//                 }
//                 else {
//                     alert("It's ok")
//                     // It's ok
//                 }
//             });
//         } else {
//             alert("Another web3 provider")
//           
//         }
//     } else {
//         
//     }    
// });
function startApp(web3) {
    var ContractAddress = "0x36b308910158f7f7aB78A29D2E3704F12139f680";
    var counter = new web3.eth.Contract(MyCounterAbi, ContractAddress);
    return counter;
  }
  
async function getMyAccounts(web3 ,c) {
    const accounts = await web3.eth.getAccounts();
    console.log(accounts[0]);
    var meAddress = accounts[0]
    console.log(meAddress);
    await c.methods.inc().send({
        from:meAddress
    })
}
  function BuyEvent() {
    if (typeof Web3 !== 'undefined') {        
      var web3 =  window.web3 = new Web3(window.web3.currentProvider)
        console.log(web3);
    } else {
        alert("It's no")
    }    
    const c = startApp(web3);
    console.log(c);
   getMyAccounts(web3,c)
    // counter(c);
    
}
    


class Eth extends React.Component {
  

    render() {      
        return (
            <div className="index-page" style={{ marginTop: "-64px" }}>
                <div className="banner" style={bannerStyle}>
                    <div>
                        {/* <a href={contractAddr}>合约地址：{contract}</a> */}

                        <Col span="5" style={colStyle}>
                        <Button type="primary" size="large" style={buttonStyle} onClick={BuyEvent}>
                        点击加数字
                        </Button>
                        {/* <div className="custom-card">
                            {counter?(counter.substr(0,counter.length>15?15:counter.length)):0}
                        </div> */}
                    </Col>
                    </div>                    
                </div>
                
            </div>
            
        );
    }
};

export default Eth;


// import React from "react";
// // import NasId from "../api/nasid";
// import { Button , Col} from "antd";
// import intl from "react-intl-universal";
// import getcontract from "../api/contractbackend.js";
// import NasId from "../api/nasid";
// import contractoption from "../api/contractoption.js"
// // import web3 from "../api/web3.js"

// import { get, inc } from "../api/contract/eth/counter.js";

// const backgroundImg = 'https://i.loli.net/2018/07/16/5b4c4a832a920.jpg'
// const buttonStyle = {
//     margin: "0.5rem"
// }
// const colStyle = {
//     padding: '0 10px'
// }
// const bannerStyle = {
//     padding: `6rem`,
//     color: `#fafafa`,
//     width: "100%", minHeight: "48rem",
//     background: `url(${backgroundImg})`, backgroundSize: 'cover'
// }
// const number = {
//     width: '150px',
//     height: '150px',
//     backgroundColor: 'darksalmon'
// }

// function IncEvent(e) {
// //     var v = parseFloat(prompt("请输入买入值"));
// //     if (v) {
// //         inc();
        
// //         inc(new Callback(function () {
// //             alert("购买成功");
// //         }, this));
// //     }
// // }

// class Eth extends React.Component {
//     constructor() {
//         super();    
//         this.state = {
//             counter: null
//         };
//     }

//     async fetchDataFromNebulas() {
//         const counter = get()
//         return { counter }
//     }

//     async componentDidMount() {
//         const {
//             counter
//         } = await this.fetchDataFromNebulas()
//         this.setState({
//             counter
//         })
//     }    

//     render() {
//         const {
//             counter
//         } = this.state        
//         return (
//             <div className="index-page" style={{ marginTop: "-64px" }}>
//                 <div className="banner" style={bannerStyle}>
//                     <div>                        
//                         <Col span="5" style={colStyle}>
//                         <Button type="primary" size="large" style={buttonStyle} onClick={IncEvent}>
//                         点击加数字
//                         </Button>
//                         <div className="custom-card">
//                             {counter?(counter.substr(0,counter.length>15?15:counter.length)):0}
//                         </div>
//                     </Col>
//                     </div>                    
//                 </div>
                
//             </div>
            
//         );
//     }
// };

// export default Eth;