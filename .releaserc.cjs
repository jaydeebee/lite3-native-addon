module.exports = {
    branches: ['main'],
    plugins: [
        [
            '@semantic-release/commit-analyzer',
            {
                preset: 'angular',
            },
        ],
        [
            '@semantic-release/release-notes-generator',
            {
                preset: 'angular',
                writerOpts: {
                    commitPartial: `* {{subject}} ([{{hash}}]({{~#if @root.host}}{{@root.host}}/{{/if}}{{@root.owner}}/{{@root.repository}}/commit/{{hash}})){{#if body}}\n\n  {{body}}{{/if}}\n`,
                },
            },
        ],
        '@semantic-release/npm',
        '@semantic-release/github',
    ],
};