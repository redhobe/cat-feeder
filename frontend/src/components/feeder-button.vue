<template>
  <button :class="classes" type="button" @click="emit('click', $event)">
    <slot>
      {{ text }}
    </slot>
  </button>
</template>

<script setup lang="ts">
import { computed } from "vue";

interface IProps {
  theme?: "primary" | "success" | "error";
  round?: boolean;
  text?: string;
}

const props = withDefaults(defineProps<IProps>(), {
  theme: "primary",
});

const emit = defineEmits<{
  (e: "click", event: MouseEvent): void;
}>();

const classes = computed<(string | Record<string, boolean>)[]>(() => [
  "button",
  `button--${props.theme}`,
  {
    "button--round": props.round,
  },
]);
</script>

<style>
.button {
  border: none;
  padding: 10px 24px;
  border-radius: var(--radius-md);
  cursor: pointer;
  font-weight: 500;
  font-size: 14px;
  line-height: 20px;
  background-color: var(--neutral-container-default);
  color: white;
}

.button--primary {
  background: var(--primary-default);
  box-shadow: none;
  border: none;
  width: 100%;
}

.button--round {
  width: 40px;
  height: 40px;
  padding: 0;
  border-radius: 50%;
}

.button--error {
  position: relative;
  color: var(--error-default);
  background: var(--error-bg);
}

.button--success {
  position: relative;
  background: var(--success-default);
}

.button--success::after {
  position: absolute;
  content: "";
  width: 32px;
  height: 32px;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  background: url("./icons/add.svg") no-repeat center;
}

.button:active {
  filter: brightness(95%);
}
</style>
