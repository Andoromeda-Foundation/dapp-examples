import { connect } from 'react-redux'
import { withRouter } from "react-router-dom";
import Home from '../pages/Home'


const mapStateToProps = ({ account }) => ({ account })

export default withRouter(
    connect(
        mapStateToProps,
        // mapDispatchToProps
    )(Home)
)
