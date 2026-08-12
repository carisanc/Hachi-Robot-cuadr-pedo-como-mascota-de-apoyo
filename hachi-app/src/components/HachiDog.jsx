import "./HachiDog.css";

export default function HachiDog({ size = 220, mode = "happy" }) {
  const colors = {
    happy: { body: "#f9c8d4", accent: "#f4a1b5", ear: "#f4a1b5", nose: "#5c3d2e", eye: "#3d2b1f" },
    calm:  { body: "#bde0f5", accent: "#8fc8e8", ear: "#8fc8e8", nose: "#3a6080", eye: "#1a3a50" },
    blind: { body: "#c3eac6", accent: "#8fd494", ear: "#8fd494", nose: "#2d6040", eye: "#1a3d28" },
  };
  const c = colors[mode] || colors.happy;

  return (
    <div className="hachi-dog-wrap">
      <svg
        width={size}
        height={size * 0.9}
        viewBox="0 0 200 180"
        xmlns="http://www.w3.org/2000/svg"
      >
        <g className="dog-body">
          {/* Shadow */}
          <ellipse cx="100" cy="172" rx="52" ry="8" fill="rgba(139,111,94,0.12)" />

          {/* Tail */}
          <g className="dog-tail" style={{ transformOrigin: "155px 85px" }}>
            <path
              d="M155 90 Q175 70 178 55 Q182 42 172 38 Q162 35 160 48 Q158 60 148 75Z"
              fill={c.accent}
              stroke={c.accent}
              strokeWidth="1"
            />
          </g>

          {/* Back legs */}
          <g className="dog-leg-bl">
            <rect x="132" y="125" width="18" height="36" rx="9" fill={c.body} />
            <rect x="128" y="153" width="24" height="10" rx="6" fill={c.accent} />
          </g>
          <g className="dog-leg-br">
            <rect x="148" y="125" width="18" height="36" rx="9" fill={c.body} />
            <rect x="144" y="153" width="24" height="10" rx="6" fill={c.accent} />
          </g>

          {/* Main body */}
          <ellipse cx="110" cy="105" rx="58" ry="38" fill={c.body} />

          {/* Front legs */}
          <g className="dog-leg-fl">
            <rect x="62" y="125" width="18" height="36" rx="9" fill={c.body} />
            <rect x="58" y="153" width="24" height="10" rx="6" fill={c.accent} />
          </g>
          <g className="dog-leg-fr">
            <rect x="78" y="125" width="18" height="36" rx="9" fill={c.body} />
            <rect x="74" y="153" width="24" height="10" rx="6" fill={c.accent} />
          </g>

          {/* Neck */}
          <ellipse cx="72" cy="92" rx="22" ry="18" fill={c.body} />

          {/* Head */}
          <ellipse cx="60" cy="72" rx="36" ry="32" fill={c.body} />

          {/* Ears */}
          <ellipse cx="38" cy="50" rx="12" ry="20" fill={c.ear} transform="rotate(-15 38 50)" />
          <ellipse cx="82" cy="50" rx="12" ry="20" fill={c.ear} transform="rotate(15 82 50)" />

          {/* Face */}
          {/* Muzzle */}
          <ellipse cx="60" cy="80" rx="18" ry="13" fill="#fff" opacity="0.6" />

          {/* Eyes */}
          <g className="dog-eye">
            <circle cx="48" cy="67" r="7" fill="#fff" />
            <circle cx="48" cy="67" r="4.5" fill={c.eye} />
            <circle cx="49.5" cy="65.5" r="1.5" fill="#fff" opacity="0.8" />
          </g>
          <g className="dog-eye">
            <circle cx="72" cy="67" r="7" fill="#fff" />
            <circle cx="72" cy="67" r="4.5" fill={c.eye} />
            <circle cx="73.5" cy="65.5" r="1.5" fill="#fff" opacity="0.8" />
          </g>

          {/* Nose */}
          <ellipse cx="60" cy="76" rx="6" ry="4" fill={c.nose} />

          {/* Smile */}
          <path
            d="M52 83 Q60 90 68 83"
            stroke={c.nose}
            strokeWidth="2.5"
            fill="none"
            strokeLinecap="round"
          />

          {/* Chest patch */}
          <ellipse cx="100" cy="95" rx="22" ry="16" fill="#fff" opacity="0.35" />

          {/* Sensor dot on forehead (ESP32 indicator) */}
          <circle cx="60" cy="52" r="4" fill={c.accent} opacity="0.7" />
          <circle cx="60" cy="52" r="2" fill="#fff" opacity="0.8" />
        </g>
      </svg>
    </div>
  );
}
