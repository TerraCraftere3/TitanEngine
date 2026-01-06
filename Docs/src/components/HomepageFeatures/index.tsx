import type {ReactNode} from 'react';
import clsx from 'clsx';
import Heading from '@theme/Heading';
import styles from './styles.module.css';
import React from 'react';

type FeatureItem = {
  title: string;
  Svg?: React.ComponentType<React.ComponentProps<'svg'>>;
  Image?: string; // <-- change to string
  description: ReactNode;
};


const FeatureList: FeatureItem[] = [
  {
    title: 'PBR Rendering',
    Image: require('@site/static/img/screenshots/helmet.png').default,
    description: (
      <>
        Get stunning visuals with Titan Engine's modern Physically Based Rendering (PBR) system
      </>
    ),
  },
  {
    title: 'Prebuilt Editor',
    Image: require('@site/static/img/screenshots/editor.png').default,
    description: (
      <>
        An easy to use prebuilt editor to help you build your games faster. (although you can also use the engine without it!)
      </>
    ),
  },
  {
    title: 'Physics',
    Image: require('@site/static/img/screenshots/physics.png').default,
    description: (
      <>
        Use PhysX and Box2D to simulate realtime physics in your games, from rigid bodies to complex collisions.
      </>
    ),
  },
];

function Feature({ title, Svg, Image, description }: FeatureItem) {
  return (
    <div className={clsx('col col--4')}>
      <div className="text--center">
        {Svg && <Svg className={styles.featureSvg} role="img" />}
        {Image && <img src={Image} className={styles.featureImage} role="img" />}
      </div>
      <div className="text--center padding-horiz--md">
        <Heading as="h3">{title}</Heading>
        <p>{description}</p>
      </div>
    </div>
  );
}

export default function HomepageFeatures(): ReactNode {
  return (
    <section className={styles.features}>
      <div className="container">
        <div className="row">
          {FeatureList.map((props, idx) => (
            <Feature key={idx} {...props} />
          ))}
        </div>
      </div>
    </section>
  );
}
