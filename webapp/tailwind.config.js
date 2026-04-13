/** @type {import('tailwindcss').Config} */
export default {
  content: ['./index.html', './src/**/*.{vue,js}'],
  darkMode: 'class',
  theme: {
    extend: {
      colors: {
        surface: {
          900: '#0f1117',
          800: '#1a1d27',
          700: '#242736',
          600: '#2e3245'
        }
      }
    }
  },
  plugins: []
}
