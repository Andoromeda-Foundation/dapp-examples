import { connect } from 'react-redux'
import { withRouter } from "react-router-dom";
import { setLanguage, setTheme, setCrypto, login } from '../actions'
import Header from '../components/Header'



const mapStateToProps = state => {
  const { lang, theme, crypto, account } = state
  return { lang, theme, crypto, account }
}

const mapDispatchToProps = dispatch => ({
  setLanguage: code => dispatch(setLanguage(code)),
  setTheme: theme => dispatch(setTheme(theme)),
  setCrypto: code => dispatch(setCrypto(code)),
  login: addr => dispatch(login(addr)),
})

export default withRouter(
  connect(
    mapStateToProps,
    mapDispatchToProps
  )(Header)
)
