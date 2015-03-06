module.exports = function(grunt) {
	grunt.loadNpmTasks('grunt-shell');
	grunt.loadNpmTasks('grunt-contrib-watch');
	grunt.initConfig({
		shell: {
			greet: {
				command: function (greeting) {
					return 'echo ' + greeting;
				}
			},
			make: {
				command: 'make',
			},
			clean: {
				command: 'make clean',
			},
			execute: {
				command: 'sudo ./mping 127.0.0.1',
			},
			test: {
				command: 'echo "TESTING: "',
			},
		},
		watch: {
			scripts: {
				files: ['**/*.c'],
					     tasks: ['run'],
					     options: {
					     spawn: false,
					     },
					     },
					     },
	});
	grunt.registerTask('default', ['watch']);
	grunt.registerTask('run', ['shell:clean', 'shell:make', 'shell:execute', 'shell:test']);
	grunt.registerTask('exec', ['shell:execute', 'shell:test']);
	grunt.registerTask('test', ['shell:test']);
}
