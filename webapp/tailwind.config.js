/** @type {import('tailwindcss').Config} */
export default {
  content: ['./index.html', './src/**/*.{vue,js}'],
  darkMode: 'class',
  theme: {
    extend: {
      colors: {
        surface: {
          900: 'var(--surface-900)',
          800: 'var(--surface-800)',
          700: 'var(--surface-700)',
          600: 'var(--surface-600)'
        },
        twhite: 'var(--text-white)',
        tgray: {
          200: 'var(--text-gray-200)',
          300: 'var(--text-gray-300)',
          400: 'var(--text-gray-400)'
        }
      }
    }
  },
  plugins: []
}
