<template>
  <div class="feeder-loader">
    <div class="loader"></div>
  </div>
</template>

<style>
.feeder-loader {
  width: 100%;
  height: 100%;
  display: flex;
  justify-content: center;
  align-items: center;
  position: absolute;
  font-size: 16px;
  font-weight: 500;
  color: var(--content-default);
  background-color: var(--bg-surface-1);
  opacity: 0.8;
  z-index: 10;
  top: 0;
  left: 0;
}

.loader {
  width: 48px;
  height: 48px;
  border: 5px solid var(--primary-default);
  border-bottom-color: transparent;
  border-radius: 50%;
  display: inline-block;
  box-sizing: border-box;
  animation: rotation 1s linear infinite;
}

@keyframes rotation {
  0% {
    transform: rotate(0deg);
  }
  100% {
    transform: rotate(360deg);
  }
}
</style>
