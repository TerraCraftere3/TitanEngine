import type {SidebarsConfig} from '@docusaurus/plugin-content-docs';

const sidebars: SidebarsConfig = {
  mainSidebar: [
    'setup',
    {
      type: 'category',
      label: 'Systems',
      items: ['systems/renderer', 'systems/physics'],
    }
  ]
};

export default sidebars;
