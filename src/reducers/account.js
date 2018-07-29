const account = (state = null, action) => {
    switch (action.type) {
        case 'login': {
            return action.data
        }
        default: return state
    }
}

export default account