import { getter, setter } from "./cookieHandler";

// Express them in One Line of code 😄
const name = 'theme'

const getUserTheme = getter({ name, defaultValue: 'light' })
const saveTheme = setter(name)

const theme = (state = getUserTheme, action) => {
    switch (action.type) {
        case 'SWITCH_TO_DARK': {
            saveTheme('dark')
            return 'dark'
        }
        case 'SWITCH_TO_LIGHT': {
            saveTheme('light')
            return 'light'
        }
        default: return state
    }
}

export default theme