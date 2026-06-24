export const registry = {
    entries: new Map(),
    register(entry) {
        this.entries.set(entry.entryName, entry);
    },
    get(entryName) {
        if (!this.entries.has(entryName)) {
            throw new Error(`Entry not found: ${entryName}`);
        }
        return this.entries.get(entryName);
    }
};

const wait = (ms) => new Promise(resolve => setTimeout(resolve, ms));

registry.register({
    entryName: "Combat.CalculateMovement",
    requiredUsage: {
        purpose: "입력된 방향키(WASD) 기반으로 캐릭터의 이동 벡터를 계산하고 정규화(Normalize)하여 위치를 갱신한다.",
        preconditions: ["캐릭터의 현재 위치가 정의되어 있어야 한다."],
        inputs: ["inputVector: 사용자 입력 벡터", "moveSpeed: 캐릭터의 기본 이동 속도", "deltaTime: 프레임 간 경과 시간"],
        outputs: ["캐릭터의 새로운 위치 좌표 (x, y)"],
        sideEffects: ["화면에 캐릭터가 이동하는 모습과, 이동 방향을 가리키는 벡터 화살표가 렌더링된다."],
        completionCondition: "지정된 목표 위치로 캐릭터가 등속 이동 애니메이션을 마치고 도달함.",
        failureConditions: ["입력 벡터의 크기가 0인 경우 (이동하지 않음)"]
    },
    async execute(runtime) {
        const { state, renderer } = runtime;
        
        // Let's simulate pressing 'W' and 'D' (Up-Right)
        const inputX = 1;
        const inputY = -1; // up is negative in canvas
        
        // Normalize vector
        const length = Math.sqrt(inputX * inputX + inputY * inputY);
        const normX = inputX / length;
        const normY = inputY / length;

        state.moveVector = { x: normX, y: normY };
        const moveSpeed = 150; // pixels per second
        const frames = 60; // 1 second animation at 60fps
        const dt = 1 / 60;

        for (let i = 0; i < frames; i++) {
            if (runtime.isPaused) await runtime.waitWhilePaused();
            
            state.character.x += normX * moveSpeed * dt;
            state.character.y += normY * moveSpeed * dt;
            
            renderer.render(state);
            await wait(16);
        }

        // Keep vector visible for explanation, but stop moving
        renderer.render(state);
    }
});

registry.register({
    entryName: "Combat.AimAtCursor",
    requiredUsage: {
        purpose: "마우스 커서의 위치를 기반으로 캐릭터가 바라봐야 할 각도를 계산하고 회전시킨다.",
        preconditions: ["캐릭터 위치와 커서 위치가 다를 때 유효하다."],
        inputs: ["characterPos: 캐릭터의 현재 중심점", "cursorPos: 마우스 커서의 위치"],
        outputs: ["캐릭터의 회전 각도 (Radians)"],
        sideEffects: ["캐릭터가 커서를 향해 부드럽게 회전하며, 조준선(Raycast 선)이 캐릭터 중심에서 커서 방향으로 그려진다."],
        completionCondition: "캐릭터가 정확히 커서 각도로 회전을 마치고 조준선이 표시됨.",
        failureConditions: ["커서가 캐릭터와 완전히 동일한 위치인 경우"]
    },
    async execute(runtime) {
        const { state, renderer } = runtime;

        // Clear previous move vector
        state.moveVector = null;

        // Set simulated cursor pos near target 1
        const target = state.targets[0];
        state.cursorPos = { x: target.x, y: target.y };
        renderer.render(state);
        await wait(500);

        // Calculate target angle
        const dx = state.cursorPos.x - state.character.x;
        const dy = state.cursorPos.y - state.character.y;
        let targetAngle = Math.atan2(dy, dx);
        
        const startAngle = state.character.rotation;
        
        // Normalize angle difference
        let diff = targetAngle - startAngle;
        while (diff < -Math.PI) diff += Math.PI * 2;
        while (diff > Math.PI) diff -= Math.PI * 2;

        const frames = 30; // half second rotation
        for (let i = 1; i <= frames; i++) {
            if (runtime.isPaused) await runtime.waitWhilePaused();
            state.character.rotation = startAngle + (diff * (i / frames));
            renderer.render(state);
            await wait(16);
        }

        state.showAimLine = true;
        renderer.render(state);
    }
});

registry.register({
    entryName: "Combat.ShootProjectile",
    requiredUsage: {
        purpose: "조준된 방향으로 투사체를 스폰하고 날려보내며, 타겟과의 충돌(Collision)을 판정한다.",
        preconditions: ["캐릭터가 타겟팅(조준)을 완료한 상태여야 한다."],
        inputs: ["spawnLocation: 투사체 생성 위치 (캐릭터 총구)", "shootAngle: 발사 각도", "targetNodes: 충돌을 검사할 적 타겟 배열"],
        outputs: ["피격당한 타겟의 상태 변화 (HP 감소 등)"],
        sideEffects: ["발사 이펙트와 함께 투사체가 궤적을 그리며 날아가고, 적과 충돌 시 파티클/색상 변화와 함께 사라진다."],
        completionCondition: "투사체가 타겟에 명중하거나 화면 밖으로 벗어남.",
        failureConditions: ["투사체 배열 생성 실패 시"]
    },
    async execute(runtime) {
        const { state, renderer } = runtime;

        state.showAimLine = false;
        state.cursorPos = null;

        const startX = state.character.x + Math.cos(state.character.rotation) * 25;
        const startY = state.character.y + Math.sin(state.character.rotation) * 25;
        
        const proj = {
            x: startX,
            y: startY,
            vx: Math.cos(state.character.rotation) * 400, // 400 pixels per sec
            vy: Math.sin(state.character.rotation) * 400
        };
        state.projectiles.push(proj);

        const dt = 1/60;
        let hit = false;
        let targetHit = null;

        while (!hit && proj.x > 0 && proj.x < state.width && proj.y > 0 && proj.y < state.height) {
            if (runtime.isPaused) await runtime.waitWhilePaused();

            proj.x += proj.vx * dt;
            proj.y += proj.vy * dt;

            // Check collision
            for (let t of state.targets) {
                if (t.hp <= 0) continue;
                const dist = Math.sqrt((t.x - proj.x)**2 + (t.y - proj.y)**2);
                if (dist <= t.radius) {
                    hit = true;
                    targetHit = t;
                    break;
                }
            }

            renderer.render(state);
            await wait(16);
        }

        // Remove projectile
        state.projectiles.pop();

        if (hit && targetHit) {
            targetHit.isHit = true;
            targetHit.hp -= 34; // 3 hits to kill
            renderer.render(state);
            await wait(200);
            targetHit.isHit = false;
            renderer.render(state);
        }
    }
});
