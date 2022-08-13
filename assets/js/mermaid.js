import mermaid from 'mermaid';

var config = {
  theme: 'default',
  fontFamily: '"Roboto", sans-serif',
};

document.addEventListener('DOMContentLoaded', () => {
  mermaid.initialize(config);
  mermaid.init(undefined, '.language-mermaid');
});
