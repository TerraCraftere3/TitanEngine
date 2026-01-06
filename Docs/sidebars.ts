import type {SidebarsConfig} from '@docusaurus/plugin-content-docs';

const sidebars: SidebarsConfig = {
  mainSidebar: [
    'setup',
    {
      type: 'category',
      label: 'Systems',
      items: [
        'systems/renderer',
        'systems/physics',
        'systems/ecs',
        'systems/input',
        'systems/asset-management',
        'systems/scene',
        'systems/scripting'
      ],
    }
  ]
};

export default sidebars;
